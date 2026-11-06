/* Simulador de escritorio del orb (Waveshare 1.75C, 466x466 redonda).
 * Ejecuta la pet_ui.c y la pet_core.c del firmware sin cambios, con una placa,
 * FreeRTOS y USB simulados. Los estados salen de eventos reales de pet_core. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zlib.h>
#include <SDL2/SDL.h>
#include "lvgl.h"
#include "board_port.h"
#include "esp_timer.h"
#include "pet_core.h"
#include "pet_ui.h"

#define RES 466

typedef enum {S_CONNECTING,S_READY,S_LISTEN,S_THINK,S_SPEAK,S_OFFLINE,S_SETTINGS,S_NETWORK,S_COUNT} scene_t;
static const struct {const char *id,*title;} scenes[S_COUNT]={
    {"1-conectando","Conectando"},{"2-lista","Lista"},{"3-escuchando","Escuchando"},{"4-pensando","Pensando"},
    {"5-hablando","Hablando"},{"6-sin-conexion","Sin conexión"},{"7-ajustes","Ajustes"},{"8-conexion-y-red","Conexión y red"},
};
static const char *reply="Hola, Ángel. Esta tarde en Madrid hará sol y unos 18 grados.";

static bool headless;
static uint64_t virtual_ms=1000;
static scene_t scene;
static pet_state_t model;
static pet_ui_view_t view;
static bool network_connected,mac_connected,demo_turn,boot_key;
static uint64_t speak_started;
static lv_obj_t *dim;

static uint64_t now_ms(void){return headless?virtual_ms:SDL_GetTicks64();}
static uint32_t tick(void){return (uint32_t)now_ms();}
int64_t esp_timer_get_time(void){return (int64_t)now_ms()*1000;}

/* Placa: el brillo y el apagado del AMOLED se imitan con una capa negra. */
static bool lock(int timeout_ms){(void)timeout_ms;return true;}
static void unlock(void){}
static void set_brightness(int pct){if(dim)lv_obj_set_style_bg_opa(dim,(lv_opa_t)(255-pct*255/100),0);}
static void panel_sleep(bool sleep){if(dim)lv_obj_set_style_bg_opa(dim,sleep?LV_OPA_COVER:LV_OPA_TRANSP,0);}
static const sim_board_t board={lock,unlock,set_brightness,panel_sleep};
const sim_board_t *pet_board(void){return &board;}

static void flush_noop(lv_display_t *display,const lv_area_t *area,uint8_t *pixels){(void)area;(void)pixels;lv_display_flush_ready(display);}
lv_display_t *pet_board_display_start(lv_indev_t **touch){
    lv_display_t *display;*touch=NULL;
    if(headless){
        static uint8_t buffer[RES*RES*2] __attribute__((aligned(64)));
        display=lv_display_create(RES,RES);
        lv_display_set_buffers(display,buffer,NULL,sizeof(buffer),LV_DISPLAY_RENDER_MODE_FULL);
        lv_display_set_flush_cb(display,flush_noop);
    }else{
        display=lv_sdl_window_create(RES,RES);
        if(!display)return NULL;
        *touch=lv_sdl_mouse_create();
    }
    lv_obj_t *top=lv_display_get_layer_top(display);
    dim=lv_obj_create(top);lv_obj_remove_style_all(dim);lv_obj_set_size(dim,RES,RES);
    lv_obj_set_style_bg_color(dim,lv_color_black(),0);lv_obj_set_style_bg_opa(dim,LV_OPA_TRANSP,0);
    lv_obj_remove_flag(dim,LV_OBJ_FLAG_CLICKABLE);
    /* Máscara redonda: un anillo negro de 100 px tapa las esquinas del cuadrado. */
    lv_obj_t *ring=lv_obj_create(top);lv_obj_remove_style_all(ring);
    lv_obj_set_size(ring,RES+200,RES+200);lv_obj_set_pos(ring,-100,-100);
    lv_obj_set_style_radius(ring,LV_RADIUS_CIRCLE,0);lv_obj_set_style_border_width(ring,100,0);
    lv_obj_set_style_border_color(ring,lv_color_black(),0);lv_obj_set_style_border_opa(ring,LV_OPA_COVER,0);
    lv_obj_remove_flag(ring,LV_OBJ_FLAG_CLICKABLE);
    return display;
}

static void post(pet_event_kind_t kind,uint64_t at,int value){
    pet_step(&model,(pet_event_t){.kind=kind,.now_ms=at,.generation=model.generation,.turn=model.turn,.value=value});
}
static void link_state(pet_connection_t state,uint64_t at){
    pet_step(&model,(pet_event_t){.kind=PET_EV_LINK,.now_ms=at,.generation=1,.value=state});
}
static void speak(uint64_t at){post(PET_EV_BUFFERED,at,0);post(PET_EV_PLAYING,at,500);speak_started=at;snprintf(view.caption,sizeof(view.caption),"%s",reply);}

static void apply_scene(scene_t next){
    scene=next;demo_turn=false;
    uint64_t now=now_ms(),start=now>4000?now-4000:0;
    pet_init(&model,start);
    post(PET_EV_USB,start,1);   /* La 1.75C de Ángel va por USB (decisión D9). */
    view.caption[0]=0;view.passkey=0;
    network_connected=next!=S_OFFLINE;mac_connected=next!=S_OFFLINE&&next!=S_CONNECTING;
    snprintf(view.wifi,sizeof(view.wifi),"%s",network_connected?"Wi-Fi conectado":"Sin Wi-Fi (funciona sin red)");
    snprintf(view.setup,sizeof(view.setup),"Desde aquí: configurar por Bluetooth");
    if(next==S_CONNECTING)link_state(PET_CONN_CONNECTING,start);
    else if(next==S_OFFLINE)link_state(PET_CONN_OFFLINE,start);
    else link_state(PET_CONN_READY,start);
    if(next==S_LISTEN)post(PET_EV_PRESS,now-600,0);
    if(next==S_THINK||next==S_SPEAK){post(PET_EV_PRESS,now-2500,0);post(PET_EV_RELEASE,now-1000,0);}
    if(next==S_SPEAK)speak(now);
    if(next==S_SETTINGS)pet_ui_bench_page(1);else if(next==S_NETWORK)pet_ui_bench_page(2);else pet_ui_home();
    if(!headless){
        char title[160];
        snprintf(title,sizeof(title),"Grok Orb 1.75C · %s  —  1-8 estados · ←/→ · B hablar · S captura · Q salir",scenes[next].title);
        lv_sdl_window_set_title(lv_display_get_default(),title);
    }
}

/* Lo que pet_app.c hace con las acciones de la interfaz, sin audio ni red. */
static void on_action(pet_ui_action_t action,void *context){
    (void)context;uint64_t now=now_ms();
    switch(action.kind){
    case PET_UI_PET:post(PET_EV_PET,now,0);break;
    case PET_UI_WAKE:model.last_input_ms=now;model.life=PET_LIFE_AWAKE;break;
    case PET_UI_VOLUME:view.volume=action.value;break;
    case PET_UI_BRIGHTNESS:view.brightness=action.value;break;
    case PET_UI_QUIET:view.quiet=action.value;post(PET_EV_QUIET,now,action.value);break;
    case PET_UI_CAPTIONS:view.captions=action.value;break;
    case PET_UI_SPEAKER:view.speaker=action.value;post(PET_EV_MUTED,now,!action.value);break;
    case PET_UI_SETUP_OPEN:post(PET_EV_SETUP,now,1);snprintf(view.setup,sizeof(view.setup),"Escribe este código al emparejar en el Mac");view.passkey=482913;break;
    case PET_UI_SETUP_CLOSE:post(PET_EV_SETUP,now,0);snprintf(view.setup,sizeof(view.setup),"Desde aquí: configurar por Bluetooth");view.passkey=0;break;
    }
}

static void refresh(void){
    uint64_t now=now_ms();
    post(PET_EV_TICK,now,0);
    if(model.dialogue==PET_DIALOGUE_SPEAK){
        /* Nivel de voz inventado para que el orb se mueva al hablar. */
        float t=(float)(now-speak_started)/1000.0f;
        post(PET_EV_PLAYING,now,(int)(450+350*sinf(t*9.0f)*sinf(t*2.3f)));
    }
    if(demo_turn&&model.dialogue==PET_DIALOGUE_WAIT&&now-model.entered_ms>=1200)speak(now);
    if(demo_turn&&model.dialogue==PET_DIALOGUE_SPEAK&&now-speak_started>=5000){
        post(PET_EV_RESPONSE_DONE,now,0);post(PET_EV_DRAIN,now,0);demo_turn=false;
    }
    if(model.dialogue==PET_DIALOGUE_LISTEN)view.caption[0]=0;
    view.pose=pet_pose(&model,now);view.dialogue=model.dialogue;view.connection=model.connection;
    view.task_channel=model.task_channel;
    view.waiting_long=model.dialogue==PET_DIALOGUE_WAIT&&now-model.entered_ms>=20000;
    snprintf(view.mac,sizeof(view.mac),"%s",mac_connected?"Puente del Mac conectado":network_connected?"Buscando el Mac emparejado":"Esperando la red");
    snprintf(view.voice,sizeof(view.voice),"%s",model.connection==PET_CONN_READY?(model.task_channel?"Voz nativa lista":"Voz lista; recuperando tareas"):"Voz nativa aún no lista");
    pet_ui_update(&view);
}

/* PNG RGBA con la misma máscara redonda que el panel, borde suavizado. */
static int write_png(const char *path,const lv_draw_buf_t *buf){
    size_t row=1+RES*4,raw_size=row*RES;unsigned char *raw=malloc(raw_size);if(!raw)return -1;
    for(int y=0;y<RES;y++){
        unsigned char *out=raw+y*row;*out++=0;
        const uint16_t *in=(const uint16_t *)(buf->data+y*buf->header.stride);
        for(int x=0;x<RES;x++){
            uint16_t p=in[x];float dx=x+.5f-RES/2.0f,dy=y+.5f-RES/2.0f,edge=RES/2.0f-sqrtf(dx*dx+dy*dy)+.5f;
            out[0]=(unsigned char)(((p>>11)&31)*255/31);out[1]=(unsigned char)(((p>>5)&63)*255/63);out[2]=(unsigned char)((p&31)*255/31);
            out[3]=(unsigned char)(edge<=0?0:edge>=1?255:edge*255);out+=4;
        }
    }
    uLongf packed_size=compressBound(raw_size);unsigned char *packed=malloc(packed_size);
    if(!packed||compress2(packed,&packed_size,raw,raw_size,9)!=Z_OK){free(raw);free(packed);return -1;}
    FILE *f=fopen(path,"wb");if(!f){free(raw);free(packed);return -1;}
    static const unsigned char signature[8]={137,80,78,71,13,10,26,10};fwrite(signature,1,8,f);
    unsigned char ihdr[13]={0,0,RES>>8,RES&255,0,0,RES>>8,RES&255,8,6,0,0,0};
    const struct {const char *type;const unsigned char *data;uint32_t size;} chunks[3]={{"IHDR",ihdr,13},{"IDAT",packed,(uint32_t)packed_size},{"IEND",NULL,0}};
    for(int i=0;i<3;i++){
        unsigned char size[4]={chunks[i].size>>24,chunks[i].size>>16,chunks[i].size>>8,chunks[i].size};fwrite(size,1,4,f);
        fwrite(chunks[i].type,1,4,f);if(chunks[i].size)fwrite(chunks[i].data,1,chunks[i].size,f);
        uLong crc=crc32(crc32(0,(const Bytef *)chunks[i].type,4),chunks[i].data,chunks[i].size);
        unsigned char c[4]={crc>>24,crc>>16,crc>>8,crc};fwrite(c,1,4,f);
    }
    int ok=fclose(f);free(raw);free(packed);return ok;
}
static int capture(const char *path){
    lv_draw_buf_t *buf=lv_snapshot_take(lv_screen_active(),LV_COLOR_FORMAT_RGB565);
    if(!buf)return -1;int result=write_png(path,buf);lv_draw_buf_destroy(buf);
    if(!result)printf("%s\n",path);
    return result;
}

static void advance(uint64_t ms){
    for(uint64_t end=virtual_ms+ms;virtual_ms<end;virtual_ms+=10){refresh();lv_timer_handler();}
}
/* Capturas fijas: tiempo virtual, sin ventana, mismo resultado en cada ejecución. */
static int shots(const char *dir){
    mkdir(dir,0755);
    for(int i=0;i<S_COUNT;i++){
        virtual_ms+=60000;apply_scene((scene_t)i);advance(1400);
        char path[512];snprintf(path,sizeof(path),"%s/%s.png",dir,scenes[i].id);
        if(capture(path))return 1;
    }
    return 0;
}

/* Una tecla que cambia de estado. La ventana y --selftest pasan por aquí. */
static void key(SDL_Scancode code,bool down){
    if(code==SDL_SCANCODE_B){
        /* B hace de botón BOOT: mantener para hablar, soltar para enviar. El bot simulado responde. */
        if(down==boot_key)return;
        boot_key=down;
        if(down){if(model.dialogue!=PET_DIALOGUE_IDLE&&model.dialogue!=PET_DIALOGUE_LISTEN)post(PET_EV_CANCEL,now_ms(),0);view.caption[0]=0;post(PET_EV_PRESS,now_ms(),0);}
        else {post(PET_EV_RELEASE,now_ms(),0);demo_turn=model.dialogue==PET_DIALOGUE_WAIT;}
        return;
    }
    if(!down)return;
    if(code>=SDL_SCANCODE_1&&code<SDL_SCANCODE_1+S_COUNT)apply_scene((scene_t)(code-SDL_SCANCODE_1));
    else if(code==SDL_SCANCODE_RIGHT||code==SDL_SCANCODE_SPACE)apply_scene((scene_t)((scene+1)%S_COUNT));
    else if(code==SDL_SCANCODE_LEFT)apply_scene((scene_t)((scene+S_COUNT-1)%S_COUNT));
    else if(code==SDL_SCANCODE_S){
        mkdir(".build",0755);mkdir(".build/sim",0755);mkdir(".build/sim/shots",0755);
        char path[256];snprintf(path,sizeof(path),".build/sim/shots/%s.png",scenes[scene].id);capture(path);
    }
    else if(code==SDL_SCANCODE_Q||code==SDL_SCANCODE_ESCAPE)exit(0);
}
static void keys(void){
    static Uint8 before[SDL_NUM_SCANCODES];int count;const Uint8 *state=SDL_GetKeyboardState(&count);
    if(count>SDL_NUM_SCANCODES)count=SDL_NUM_SCANCODES;
    for(int code=0;code<count;code++)if(state[code]!=before[code])key((SDL_Scancode)code,state[code]);
    memcpy(before,state,count);
}

/* Recorre los controles con tiempo virtual y comprueba lo que hace pet_core. */
static int failures;
static void check(bool ok,const char *what){printf("%s %s\n",ok?"ok   ":"FALLO",what);if(!ok)failures++;}
static void tap(SDL_Scancode code){key(code,true);key(code,false);advance(300);}
static int selftest(void){
    apply_scene(S_READY);advance(500);
    check(model.connection==PET_CONN_READY&&model.dialogue==PET_DIALOGUE_IDLE,"2 lista: conectada y en reposo");
    key(SDL_SCANCODE_B,true);advance(800);check(model.dialogue==PET_DIALOGUE_LISTEN,"B mantenida: escuchando");
    key(SDL_SCANCODE_B,false);advance(100);check(model.dialogue==PET_DIALOGUE_WAIT,"B soltada: pensando");
    advance(1300);check(model.dialogue==PET_DIALOGUE_SPEAK&&view.caption[0],"el bot simulado responde con subtítulo");
    advance(5200);check(model.dialogue==PET_DIALOGUE_IDLE,"la respuesta termina y vuelve al reposo");
    key(SDL_SCANCODE_B,true);advance(300);key(SDL_SCANCODE_B,false);advance(1500);
    key(SDL_SCANCODE_B,true);advance(300);check(model.dialogue==PET_DIALOGUE_LISTEN&&!view.caption[0],"B durante la respuesta interrumpe y vuelve a escuchar");
    key(SDL_SCANCODE_B,false);advance(7000);
    static const struct {pet_connection_t connection;pet_dialogue_t dialogue;} expected[S_COUNT]={
        {PET_CONN_CONNECTING,PET_DIALOGUE_IDLE},{PET_CONN_READY,PET_DIALOGUE_IDLE},{PET_CONN_READY,PET_DIALOGUE_LISTEN},
        {PET_CONN_READY,PET_DIALOGUE_WAIT},{PET_CONN_READY,PET_DIALOGUE_SPEAK},{PET_CONN_OFFLINE,PET_DIALOGUE_IDLE},
        {PET_CONN_READY,PET_DIALOGUE_IDLE},{PET_CONN_READY,PET_DIALOGUE_IDLE}};
    for(int i=0;i<S_COUNT;i++){
        tap((SDL_Scancode)(SDL_SCANCODE_1+i));char what[96];snprintf(what,sizeof(what),"tecla %d: %s",i+1,scenes[i].title);
        check(scene==i&&model.connection==expected[i].connection&&model.dialogue==expected[i].dialogue,what);
    }
    tap(SDL_SCANCODE_RIGHT);check(scene==S_CONNECTING,"→ desde el último estado vuelve al primero");
    tap(SDL_SCANCODE_LEFT);check(scene==S_NETWORK,"← desde el primero va al último");
    tap(SDL_SCANCODE_SPACE);check(scene==S_CONNECTING,"Espacio avanza");
    tap(SDL_SCANCODE_3);advance(3000);check(model.dialogue==PET_DIALOGUE_LISTEN,"3 escuchando se mantiene sin tocar B");
    printf(failures?"%d comprobaciones fallidas\n":"Todo bien\n",failures);
    return failures?1:0;
}

int main(int argc,char **argv){
    const char *shot_dir=NULL;bool test=false;
    for(int i=1;i<argc;i++){if(!strcmp(argv[i],"--shots")&&i+1<argc)shot_dir=argv[++i];else if(!strcmp(argv[i],"--selftest"))test=true;}
    headless=shot_dir!=NULL||test;
    lv_init();lv_tick_set_cb(tick);
    view.volume=60;view.brightness=80;view.battery=-1;view.quiet=true;view.captions=true;view.speaker=true;
    if(pet_ui_start(on_action,NULL)!=ESP_OK){fprintf(stderr,"No se pudo arrancar la interfaz\n");return 1;}
    if(test)return selftest();
    if(headless)return shots(shot_dir);
    apply_scene(S_READY);
    printf("Grok Orb 1.75C\n  1-8  estados (conectando, lista, escuchando, pensando, hablando, sin conexión, ajustes, red)\n"
           "  ←/→  estado anterior/siguiente   B  mantener para hablar   S  captura   Q  salir\n"
           "  Ratón: tocar = caricia, deslizar hacia arriba o pulsar la barra inferior = ajustes\n");
    for(;;){
        uint32_t idle=lv_timer_handler();keys();refresh();
        SDL_Delay(idle<1?1:idle>10?10:idle);
    }
}
