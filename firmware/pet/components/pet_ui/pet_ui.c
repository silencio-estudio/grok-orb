#include "pet_ui.h"
#include "pet_strings.h"
#include "pet_motion.h"
#include "board_port.h"
#include "lvgl.h"
#include "esp_timer.h"
#include "driver/usb_serial_jtag.h"
#include "mbedtls/base64.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#if defined(CONFIG_PET_LANG_ZH) && CONFIG_PET_LANG_ZH
extern const lv_font_t muse_font_cjk_16;
#endif

/* Tema elegido en menuconfig (Grok pet interface > Theme). El suave es el de siempre;
 * el de 8 bits usa cuatro colores del orb actual, Unifont y un orb de bloques de 8 px. */
#if defined(CONFIG_PET_THEME_PIXEL) && CONFIG_PET_THEME_PIXEL
#define PIXEL_THEME 1
extern const lv_font_t pet_font_pixel_16;
extern const lv_font_t pet_font_title_16;
/* Paleta de 16 colores al estilo PICO-8; la interfaz solo usa estos índices. */
static const uint32_t palette[16]={
    0x000000,0x1d2b53,0x7e2553,0x008751,0xab5236,0x5f574f,0xc2c3c7,0xfff1e8,
    0xff004d,0xffa300,0xffec27,0x00e436,0x29adff,0x83769c,0xff77a8,0xffccaa};
#define C_BG      0x000000   /* 0 */
#define C_ORB     0xff77a8   /* 14 */
#define C_LIGHT   0xfff1e8   /* 7 */
#define C_TEXT    0xfff1e8   /* 7 */
#define C_HINT    0x83769c   /* 13 */
#define C_PANEL   0x000000   /* 0 */
#define C_BUTTON  0x1d2b53   /* 1 */
#define C_TRACK   0x1d2b53   /* 1 */
#define C_EYE     0x000000   /* 0 */
#define C_BAR     0x83769c   /* 13 */
#define C_OFFLINE 0xc2c3c7   /* 6 */
/* Rampas de sombra a luz del orb, por índice de paleta. */
static const uint8_t ramp_orb[4]={2,8,14,7},ramp_offline[4]={5,13,6,7};
#define RADIUS(r) 0
#else
#define PIXEL_THEME 0
extern const lv_font_t pet_font_latin_16;
#define C_BG      0x101010
#define C_ORB     0xfa2c99
#define C_LIGHT   0xefc9dd
#define C_TEXT    0xd7cdd3
#define C_HINT    0x8b7f87
#define C_PANEL   0x151215
#define C_BUTTON  0x32272e
#define C_TRACK   0x4b3341
#define C_EYE     0x121212
#define C_BAR     0x494146
#define C_OFFLINE 0x6b6267
#define RADIUS(r) (r)
#endif
static lv_font_t font,latin,title_font;
static SemaphoreHandle_t view_lock,ready_signal;
static pet_ui_view_t view;
static pet_ui_callback_t callback;
static void *callback_context;
static lv_obj_t *root,*body,*caption,*handle,*drawer,*connection_page;
#if !PIXEL_THEME
static lv_obj_t *eyes[2],*smiles[2];   /* en el tema de 8 bits los ojos van dibujados en el lienzo */
#endif
static lv_obj_t *volume_slider,*brightness_slider,*quiet_switch,*captions_switch,*speaker_switch;
static lv_obj_t *network_label,*bridge_label,*voice_label,*setup_label,*passkey_label,*header_labels[2];
static bool initialized,screen_on=true,syncing;
/* Últimos valores enviados: los estilos de transformación de LVGL invalidan el objeto
 * aunque no cambien, y el brillo es un comando por QSPI. Valores imposibles = aún no enviado. */
static int last_brightness=-1;
#if !PIXEL_THEME
static int last_sx=-1,last_sy=-1,last_rotation=-100000,last_mix=-1,last_eye_h=-1;
#endif
/* Pose suavizada: pet_core da el objetivo de cada estado y los muelles lo siguen.
 * breath (0..1) mezcla la respiración de reposo, que así entra y sale sin saltos. */
static pet_spring_t s_scale_x,s_scale_y,s_offset,s_rotation,s_eye,s_look,s_breath;
static pet_tween_t offline_tween;
static int64_t last_us;
#if PIXEL_THEME
#define PIXEL_BLOCKS 60   /* 480 px: contiene el círculo de 466 px, con margen para el tramado */
#define PIXEL_FRAME_US 125000   /* 8 fotogramas por segundo */
static uint8_t pixel_buf[PIXEL_BLOCKS*PIXEL_BLOCKS*4] __attribute__((aligned(4)));
static uint8_t pixel_next[sizeof(pixel_buf)];
static int64_t pixel_frame=-1;
#endif
static int page,requested_page=-1;
static bool snapshot_requested;
static int touch_x,touch_y;
static bool touching;
static char last_caption[321],last_hint[64];
static int64_t lost_at;
static bool offline_shown;
static lv_obj_t *voice_hint;
static void action(pet_ui_action_kind_t kind,int value){if(callback)callback((pet_ui_action_t){kind,value},callback_context);}
static lv_obj_t *label(lv_obj_t *parent,const char *text,int width){
    lv_obj_t *o=lv_label_create(parent);lv_obj_set_style_text_font(o,&font,0);
    lv_obj_set_style_text_color(o,lv_color_hex(C_TEXT),0);lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);
    if(width){lv_obj_set_width(o,width);}
    lv_label_set_text(o,text);return o;
}
static void show_page(int next){
    page=next;lv_obj_set_flag(drawer,LV_OBJ_FLAG_HIDDEN,next!=1);lv_obj_set_flag(connection_page,LV_OBJ_FLAG_HIDDEN,next!=2);
}
static void touch(lv_event_t *event){
    lv_event_code_t code=lv_event_get_code(event);lv_point_t point;
    lv_indev_t *indev=lv_indev_active();if(!indev)return;lv_indev_get_point(indev,&point);
    if(code==LV_EVENT_PRESSED){touch_x=point.x;touch_y=point.y;touching=true;action(PET_UI_WAKE,0);}
    else if(code==LV_EVENT_RELEASED&&touching){
        touching=false;pet_gesture_t g=pet_gesture(touch_x,touch_y,point.x,point.y);
        if(g==PET_GESTURE_SETTINGS)show_page(1);
        else if(g==PET_GESTURE_TAP||g==PET_GESTURE_STROKE)action(PET_UI_PET,0);
    }else if(code==LV_EVENT_PRESS_LOST)touching=false;
}
static void open_drawer(lv_event_t *event){(void)event;show_page(1);action(PET_UI_WAKE,0);}
static void home(lv_event_t *event){(void)event;show_page(0);action(PET_UI_WAKE,0);}
static void connections(lv_event_t *event){(void)event;show_page(2);action(PET_UI_WAKE,0);}
static void provision(lv_event_t *event){(void)event;action(PET_UI_SETUP_OPEN,0);}
static void close_provision(lv_event_t *event){(void)event;action(PET_UI_SETUP_CLOSE,0);}
static lv_obj_t *button(lv_obj_t *parent,const char *text,int width,lv_event_cb_t cb){
    lv_obj_t *o=lv_button_create(parent);lv_obj_set_size(o,width,42);lv_obj_set_style_bg_color(o,lv_color_hex(C_BUTTON),0);
    lv_obj_set_style_radius(o,RADIUS(21),0);lv_obj_set_style_shadow_width(o,0,0);
#if PIXEL_THEME
    lv_obj_set_style_border_width(o,2,0);lv_obj_set_style_border_color(o,lv_color_hex(C_ORB),0);
#endif
    lv_obj_t *txt=label(o,text,0);lv_obj_center(txt);
    lv_obj_add_event_cb(o,cb,LV_EVENT_CLICKED,NULL);return o;
}
static void slider_changed(lv_event_t *event){
    if(syncing)return;
    lv_obj_t *o=lv_event_get_target(event);
    if(lv_event_get_code(event)==LV_EVENT_VALUE_CHANGED)action(o==volume_slider?PET_UI_VOLUME:PET_UI_BRIGHTNESS,lv_slider_get_value(o));
}
static lv_obj_t *slider(lv_obj_t *parent,const char *text,int y,int min,int max){
    lv_obj_t *txt=label(parent,text,220);lv_obj_set_style_text_align(txt,LV_TEXT_ALIGN_LEFT,0);lv_obj_set_pos(txt,123,y);
    lv_obj_t *s=lv_slider_create(parent);lv_obj_set_size(s,220,8);lv_obj_set_pos(s,123,y+33);
    lv_slider_set_range(s,min,max);lv_obj_set_style_bg_color(s,lv_color_hex(C_TRACK),LV_PART_MAIN);
    lv_obj_set_style_bg_color(s,lv_color_hex(C_ORB),LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(s,lv_color_hex(C_LIGHT),LV_PART_KNOB);
#if PIXEL_THEME
    lv_obj_set_style_radius(s,0,LV_PART_MAIN);lv_obj_set_style_radius(s,0,LV_PART_INDICATOR);lv_obj_set_style_radius(s,0,LV_PART_KNOB);
#endif
    lv_obj_add_event_cb(s,slider_changed,LV_EVENT_VALUE_CHANGED,NULL);return s;
}
static void switch_changed(lv_event_t *event){
    if(syncing)return;
    lv_obj_t *o=lv_event_get_target(event);
    pet_ui_action_kind_t kind=o==quiet_switch?PET_UI_QUIET:o==captions_switch?PET_UI_CAPTIONS:PET_UI_SPEAKER;
    action(kind,lv_obj_has_state(o,LV_STATE_CHECKED));
}
static lv_obj_t *toggle(lv_obj_t *parent,const char *text,int y){
    lv_obj_t *txt=label(parent,text,160);lv_obj_set_style_text_align(txt,LV_TEXT_ALIGN_LEFT,0);lv_obj_set_pos(txt,123,y+4);
    /* Borde derecho del interruptor (295+48) alineado con el de los deslizadores (123+220). */
    lv_obj_t *o=lv_switch_create(parent);lv_obj_set_size(o,48,25);lv_obj_set_pos(o,295,y);
    lv_obj_set_style_bg_color(o,lv_color_hex(C_ORB),LV_PART_INDICATOR|LV_STATE_CHECKED);
#if PIXEL_THEME
    lv_obj_set_style_bg_color(o,lv_color_hex(C_TRACK),LV_PART_MAIN);lv_obj_set_style_bg_color(o,lv_color_hex(C_LIGHT),LV_PART_KNOB);
    lv_obj_set_style_radius(o,0,LV_PART_MAIN);lv_obj_set_style_radius(o,0,LV_PART_INDICATOR);lv_obj_set_style_radius(o,0,LV_PART_KNOB);
#endif
    lv_obj_add_event_cb(o,switch_changed,LV_EVENT_VALUE_CHANGED,NULL);return o;
}
static lv_obj_t *overlay(void){
    lv_obj_t *o=lv_obj_create(root);lv_obj_remove_style_all(o);lv_obj_set_size(o,466,466);
    lv_obj_set_style_bg_color(o,lv_color_hex(C_PANEL),0);lv_obj_set_style_bg_opa(o,LV_OPA_COVER,0);
    lv_obj_remove_flag(o,LV_OBJ_FLAG_SCROLLABLE);lv_obj_add_flag(o,LV_OBJ_FLAG_HIDDEN);return o;
}
static void set_checked(lv_obj_t *o,bool on){lv_obj_set_state(o,LV_STATE_CHECKED,on);}
static void snapshot(void){
#if LV_USE_SNAPSHOT
    /* A single UI-task operation: no caption or pairing code may enter exported pixels. */
    bool caption_hidden=lv_obj_has_flag(caption,LV_OBJ_FLAG_HIDDEN);
    bool code_hidden=lv_obj_has_flag(passkey_label,LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(caption,LV_OBJ_FLAG_HIDDEN);lv_obj_add_flag(passkey_label,LV_OBJ_FLAG_HIDDEN);
    lv_draw_buf_t *buf=lv_snapshot_take(root,LV_COLOR_FORMAT_RGB565);
    lv_obj_set_flag(caption,LV_OBJ_FLAG_HIDDEN,caption_hidden);lv_obj_set_flag(passkey_label,LV_OBJ_FLAG_HIDDEN,code_hidden);
    if(!buf)return;
    char line[48];int count=snprintf(line,sizeof(line),"\nSNAP BEGIN %u %u 144\n",(unsigned)buf->header.w,(unsigned)buf->header.h);
    usb_serial_jtag_write_bytes(line,count,portMAX_DELAY);
    unsigned char encoded[196];
    for(uint32_t y=0;y<buf->header.h;y++){
        const uint8_t *p=buf->data+y*buf->header.stride;size_t left=buf->header.w*2;
        while(left){size_t n=left>144?144:left,out=0;mbedtls_base64_encode(encoded,sizeof(encoded)-1,&out,p,n);encoded[out++]='\n';usb_serial_jtag_write_bytes(encoded,out,portMAX_DELAY);left-=n;p+=n;}
    }
    usb_serial_jtag_write_bytes("SNAP END\n",9,portMAX_DELAY);lv_draw_buf_destroy(buf);
#endif
}
#define CALM_PERIOD_MS 55   /* ≈18 fps para la respiración en reposo */
static void motion_init(void){
    /* Muelles críticos (sin rebote): un cambio de estado recorre el 95 % del camino en
     * 320 ms con forma de S; los ojos, en 110 ms; la respiración entra y sale en 400 ms. */
    pet_spring_init_settle(&s_scale_x,1,.32f);pet_spring_init_settle(&s_scale_y,1,.32f);
    pet_spring_init_settle(&s_offset,0,.32f);pet_spring_init_settle(&s_rotation,0,.32f);
    pet_spring_init_settle(&s_eye,1,.11f);pet_spring_init_settle(&s_look,0,.32f);pet_spring_init_settle(&s_breath,0,.4f);
}
/* Sigue la pose de pet_core con los muelles. En reposo pet_core sube y baja el orb a
 * saltos de 1 px; aquí se cambia por una respiración continua de escala. */
static void motion_step(const pet_pose_t *p,int64_t now_us){
    float dt=last_us?(float)(now_us-last_us)/1e6f:0;last_us=now_us;
    bool idle=p->expression==PET_EXPR_IDLE;
    s_scale_x.target=idle?1:p->scale_x;s_scale_y.target=idle?1:p->scale_y;s_offset.target=idle?0:p->offset_y;
    s_rotation.target=p->rotation;s_eye.target=p->eye_open;s_look.target=p->look_x;s_breath.target=idle?1:0;
    pet_spring_t *all[]={&s_scale_x,&s_scale_y,&s_offset,&s_rotation,&s_eye,&s_look,&s_breath};
    for(unsigned i=0;i<sizeof(all)/sizeof(all[0]);i++)pet_spring_step(all[i],dt);
}
/* Respiración: ciclo de 4 s, solo escala. La fase se toma sobre su propio periodo para
 * que no salte al dar la vuelta ningún contador. Es un seno: ease-in-out por sí mismo. */
#define BREATH_PERIOD_US 4000000
static float breath(int64_t now_us){return s_breath.value*sinf((float)(now_us%BREATH_PERIOD_US)/BREATH_PERIOD_US*6.2831853f);}
/* Uniform inward breathing: the largest idle silhouette is the inscribed circle.
 * Blend through the existing spring so entering/leaving idle stays continuous. */
static float idle_scale(int64_t now_us){
    float wave=(1+sinf((float)(now_us%BREATH_PERIOD_US)/BREATH_PERIOD_US*6.2831853f))*.5f;
    return (466.0f/224.0f)*(1-.04f*wave);
}
/* Mezcla entre el color del orb y el de sin conexión, con --ease-io3 (Cubic) en 320 ms. */
static int offline_mix(bool offline,int64_t now_us){
    float now=(float)(now_us%600000000)/1e6f;
    if(offline!=offline_shown){
        offline_shown=offline;pet_tween_start_curve(&offline_tween,PET_EASE_IO3,pet_tween_value(&offline_tween,now),offline?1:0,now,.32f);
    }
    return (int)lroundf(pet_tween_value(&offline_tween,now)*255);
}
#if PIXEL_THEME
static void put_block(uint8_t *buf,int x,int y,uint32_t rgb){
    if(x<0||y<0||x>=PIXEL_BLOCKS||y>=PIXEL_BLOCKS)return;
    uint8_t *px=buf+(y*PIXEL_BLOCKS+x)*4;px[0]=rgb&0xff;px[1]=(rgb>>8)&0xff;px[2]=(rgb>>16)&0xff;px[3]=0xff;
}
/* Tramado ordenado Bayer 4×4 (umbrales de 0 a 15). */
static const uint8_t bayer[4][4]={{0,8,2,10},{12,4,14,6},{3,11,1,9},{15,7,13,5}};
/* Orb de bloques: elipse rotada con luz desde arriba a la izquierda, sombreada con una rampa
 * de 4 colores de la paleta y tramado entre ellos; ojos de 3 bloques. offline (0..255)
 * mezcla con tramado la rampa rosa y la gris. */
static void pixel_orb(uint8_t *buf,float sx,float sy,float offset,float rotation,float eye,float look,bool smile,int offline){
    memset(buf,0,sizeof(pixel_buf));
    float c=PIXEL_BLOCKS/2.0f,cy=c+offset/8,rx=13.4f*sx,ry=13.4f*sy,a=-rotation*3.14159265f/180,ca=cosf(a),sa=sinf(a);
    for(int y=0;y<PIXEL_BLOCKS;y++)for(int x=0;x<PIXEL_BLOCKS;x++){
        float dx0=x+.5f-c,dy0=y+.5f-cy,dx=dx0*ca-dy0*sa,dy=dx0*sa+dy0*ca;
        float edge_x=fabsf(dx0)*8+4,edge_y=fabsf(y+.5f-c)*8+4;
        if(edge_x*edge_x+edge_y*edge_y>233*233)continue;
        if((dx*dx)/(rx*rx)+(dy*dy)/(ry*ry)>1)continue;
        float nx=dx/rx,ny=dy/ry,nz=sqrtf(fmaxf(0,1-nx*nx-ny*ny));
        float light=fmaxf(0,-.45f*nx-.6f*ny+.66f*nz);   /* luz normalizada (-0,45, -0,6, 0,66) */
        /* Rosa de base, sombra tramada abajo a la derecha y blanco solo en el brillo especular. */
        float spec=light*light;spec*=spec;spec*=spec;
        float v=.9f+1.3f*light+1.2f*spec,threshold=(bayer[y&3][x&3]+.5f)/16;
        int step=(int)v;if(step>2)step=2;if(v-step>threshold)step++;
        const uint8_t *ramp=offline*16>(bayer[y&3][x&3]+.5f)*255?ramp_offline:ramp_orb;
        put_block(buf,x,y,palette[ramp[step]]);
    }
    /* Centros de los ojos en bloques desde el centro del orb (los del orb suave entre 8). */
    const float ex[2]={-5.75f,2.1f},ey=-2.1f;
    for(int i=0;i<2;i++){
        float cx0=ex[i]*sx+look/8,cy0=ey*sy;
        int bx=(int)floorf(c+cx0*ca+cy0*sa),by=(int)floorf(cy-cx0*sa+cy0*ca);
        if(smile){const int arc[5][2]={{0,-1},{-1,0},{1,0},{-2,1},{2,1}};for(int k=0;k<5;k++)put_block(buf,bx+arc[k][0],by+arc[k][1],C_EYE);continue;}
        int half=(int)lroundf(fmaxf(.5f,3.75f*eye));
        for(int dy=-half;dy<half;dy++)for(int dx=-1;dx<=1;dx++)put_block(buf,bx+dx,by+dy,C_EYE);
    }
}
#endif
static void draw(lv_timer_t *timer){
    pet_ui_view_t current;
    xSemaphoreTake(view_lock,portMAX_DELAY);current=view;
    int next=requested_page;requested_page=-1;bool snap=snapshot_requested;snapshot_requested=false;
    xSemaphoreGive(view_lock);
    if(next>=0)show_page(next);
    pet_pose_t p=current.pose;
    if(p.screen_on!=screen_on){
        screen_on=p.screen_on;pet_board()->panel_sleep(!screen_on);
        if(screen_on){lv_obj_invalidate(root);last_brightness=-1;}
    }
    if(!screen_on){if(snap)snapshot();return;}
    int brightness=p.dim?current.brightness/4:current.brightness;
    if(brightness<5)brightness=5;
    if(brightness!=last_brightness){last_brightness=brightness;pet_board()->set_brightness(brightness);}
    int64_t now_us=esp_timer_get_time();
    motion_step(&p,now_us);
    /* Encadenado: la sonrisa (feliz o dormido) aparece cuando los ojos ya se han cerrado. */
    bool smile=(p.expression==PET_EXPR_HAPPY||p.expression==PET_EXPR_SLEEP)&&s_eye.value<.35f;
    if(current.connection==PET_CONN_READY)lost_at=0;else if(!lost_at)lost_at=now_us;
    /* Sin conexión solo tras 10 s seguidos sin enlace: al arrancar y en cada reintento
     * pet_link alterna CONNECTING y OFFLINE, y el orb no debe parpadear. */
    bool offline=lost_at&&now_us-lost_at>=10000000;
    int mix=offline_mix(offline,now_us);
#if PIXEL_THEME
    /* Pocos fotogramas: el orb de bloques solo se recalcula 8 veces por segundo. */
    int64_t frame=now_us/PIXEL_FRAME_US;
    if(frame!=pixel_frame){
        pixel_frame=frame;float b=breath(now_us);
        float idle=fminf(1,fmaxf(0,s_breath.value)),grow=idle*(idle_scale(now_us)*112.0f/107.2f-1);
        lv_obj_set_y(body,210-PIXEL_BLOCKS/2+(int)lroundf(23*idle));
        pixel_orb(pixel_next,s_scale_x.value+grow-.025f*b*(1-idle),s_scale_y.value+grow+.055f*b*(1-idle),s_offset.value,s_rotation.value,s_eye.value,s_look.value,smile,mix);
        if(memcmp(pixel_next,pixel_buf,sizeof(pixel_buf))){memcpy(pixel_buf,pixel_next,sizeof(pixel_buf));lv_obj_invalidate(body);}
    }
#else
    uint32_t color=lv_color_to_u32(lv_color_mix(lv_color_hex(C_OFFLINE),lv_color_hex(C_ORB),(uint8_t)mix))&0xffffff;
    float b=breath(now_us);
    float idle=fminf(1,fmaxf(0,s_breath.value)),grow=idle*(idle_scale(now_us)-1);
    /* Escala con transform_scale (1/256): el borde se mueve por debajo del píxel. */
    int sx=(int)lroundf((s_scale_x.value+grow-.010f*b*(1-idle))*256),sy=(int)lroundf((s_scale_y.value+grow+.025f*b*(1-idle))*256);
    /* Never round the maximum idle diameter beyond the physical circle. */
    if(idle>.999f){if(sx>532)sx=532;if(sy>532)sy=532;}
    int rotation=(int)lroundf(s_rotation.value*10);
    lv_obj_set_y(body,98+(int)lroundf(s_offset.value+23*idle));
    if(sx!=last_sx){last_sx=sx;lv_obj_set_style_transform_scale_x(body,sx,0);}
    if(sy!=last_sy){last_sy=sy;lv_obj_set_style_transform_scale_y(body,sy,0);}
    if(rotation!=last_rotation){last_rotation=rotation;lv_obj_set_style_transform_rotation(body,rotation,0);}
    if(mix!=last_mix){last_mix=mix;lv_obj_set_style_bg_color(body,lv_color_hex(color),0);}
    /* Parpadeo hacia el centro del ojo, no hacia arriba. */
    int eye_h=(int)lroundf(fmaxf(5,60*s_eye.value));
    for(unsigned i=0;i<2;i++){
        int x=(int)lroundf(224*(i?.5223f:.2411f)+s_look.value);
        lv_obj_set_pos(eyes[i],x,95-eye_h/2);lv_obj_set_height(eyes[i],eye_h);
        if(eye_h!=last_eye_h)lv_obj_set_style_transform_pivot_y(eyes[i],eye_h/2,0);
        lv_obj_set_flag(eyes[i],LV_OBJ_FLAG_HIDDEN,smile);
        lv_obj_set_pos(smiles[i],x-6,65);lv_obj_set_flag(smiles[i],LV_OBJ_FLAG_HIDDEN,!smile);
    }
    last_eye_h=eye_h;
#endif
    const char *text=current.captions?current.caption:"";
    if(current.dialogue==PET_DIALOGUE_LISTEN)text=PET_STR_LISTEN;
    else if(current.dialogue==PET_DIALOGUE_WAIT)text=current.waiting_long?PET_STR_WAIT_LONG:PET_STR_WAIT;
    else if(p.expression==PET_EXPR_HAPPY)text=PET_STR_PETTED;
    else if(p.expression==PET_EXPR_GREET&&p.invitation)text=PET_STR_GREET;
    else if(current.dialogue==PET_DIALOGUE_IDLE)text="";
    if(strcmp(last_caption,text)){snprintf(last_caption,sizeof(last_caption),"%s",text);lv_label_set_text(caption,last_caption);}
    lv_obj_set_flag(caption,LV_OBJ_FLAG_HIDDEN,!text[0]);
    const char *hint=current.connection==PET_CONN_READY?PET_STR_HINT_TALK:offline?PET_STR_OFFLINE:PET_STR_HINT_CONNECTING;
    if(current.dialogue==PET_DIALOGUE_LISTEN)hint=PET_STR_HINT_RELEASE;
    else if(current.connection==PET_CONN_READY&&current.dialogue!=PET_DIALOGUE_IDLE)hint=PET_STR_HINT_INTERRUPT;
    if(strcmp(last_hint,hint)){snprintf(last_hint,sizeof(last_hint),"%s",hint);lv_label_set_text(voice_hint,last_hint);}
    if(page==1){
        syncing=true;lv_slider_set_value(volume_slider,current.volume,LV_ANIM_OFF);
        lv_slider_set_value(brightness_slider,current.brightness,LV_ANIM_OFF);
        set_checked(quiet_switch,current.quiet);set_checked(captions_switch,current.captions);set_checked(speaker_switch,current.speaker);syncing=false;
    }
    if(page==2){
        lv_label_set_text(network_label,current.wifi);lv_label_set_text(bridge_label,current.mac);lv_label_set_text(voice_label,current.voice);
        lv_label_set_text(setup_label,current.setup);
        char code[32];if(current.passkey)snprintf(code,sizeof(code),PET_STR_PASSKEY_FMT,current.passkey);else code[0]=0;
        lv_label_set_text(passkey_label,code);
    }
    /* Cabecera de Ajustes y de Conexión: el nombre del orb y, si hay batería, su nivel.
     * La pantalla principal no lo lleva. */
    if(page){
        char header[64];
        if(current.battery>=0)snprintf(header,sizeof(header),PET_STR_HEADER_BATTERY_FMT,PET_DISPLAY_NAME,current.battery);
        else snprintf(header,sizeof(header),"%s",PET_DISPLAY_NAME);
        lv_obj_t *target=header_labels[page-1];
        if(strcmp(lv_label_get_text(target),header))lv_label_set_text(target,header);
    }
    /* Reposo: solo respira. El dibujo baja a unos 18 fps; con cualquier cambio vuelve
     * al refresco normal en el siguiente fotograma. */
    bool calm=p.expression==PET_EXPR_IDLE&&current.dialogue==PET_DIALOGUE_IDLE&&!touching&&
        pet_spring_settled(&s_scale_x,.002f)&&pet_spring_settled(&s_scale_y,.002f)&&pet_spring_settled(&s_offset,.2f)&&
        pet_spring_settled(&s_rotation,.05f)&&pet_spring_settled(&s_eye,.01f)&&pet_spring_settled(&s_look,.2f)&&
        pet_spring_settled(&s_breath,.01f)&&(mix==0||mix==255);
    static uint32_t draw_period=LV_DEF_REFR_PERIOD;
    uint32_t period=calm?CALM_PERIOD_MS:LV_DEF_REFR_PERIOD;
    if(timer&&period!=draw_period){draw_period=period;lv_timer_set_period(timer,period);}
    if(snap)snapshot();
}
static void build(void){
    root=lv_screen_active();lv_obj_remove_style_all(root);lv_obj_set_style_bg_color(root,lv_color_hex(C_BG),0);
    lv_obj_set_style_bg_opa(root,LV_OPA_COVER,0);lv_obj_remove_flag(root,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(root,touch,LV_EVENT_ALL,NULL);
    motion_init();
#if PIXEL_THEME
    /* Bloques ampliados ×8 sin suavizado: cada bloque es un cuadrado de 8 px. */
    body=lv_canvas_create(root);lv_canvas_set_buffer(body,pixel_buf,PIXEL_BLOCKS,PIXEL_BLOCKS,LV_COLOR_FORMAT_ARGB8888);
    lv_obj_set_pos(body,233-PIXEL_BLOCKS/2,210-PIXEL_BLOCKS/2);lv_image_set_scale(body,256*8);lv_image_set_antialias(body,false);
    lv_obj_add_flag(body,LV_OBJ_FLAG_EVENT_BUBBLE);lv_obj_remove_flag(body,LV_OBJ_FLAG_SCROLLABLE);
#else
    body=lv_obj_create(root);lv_obj_remove_style_all(body);lv_obj_set_size(body,224,224);lv_obj_set_pos(body,121,98);
    lv_obj_set_style_radius(body,LV_RADIUS_CIRCLE,0);lv_obj_set_style_bg_color(body,lv_color_hex(C_ORB),0);
    lv_obj_set_style_transform_pivot_x(body,112,0);lv_obj_set_style_transform_pivot_y(body,112,0);
    lv_obj_set_style_bg_opa(body,LV_OPA_COVER,0);lv_obj_add_flag(body,LV_OBJ_FLAG_EVENT_BUBBLE);lv_obj_remove_flag(body,LV_OBJ_FLAG_SCROLLABLE);
    for(unsigned i=0;i<2;i++){
        eyes[i]=lv_obj_create(body);lv_obj_remove_style_all(eyes[i]);lv_obj_set_size(eyes[i],24,60);
        lv_obj_set_style_radius(eyes[i],12,0);lv_obj_set_style_bg_color(eyes[i],lv_color_hex(C_EYE),0);
        lv_obj_set_style_bg_opa(eyes[i],LV_OPA_COVER,0);lv_obj_remove_flag(eyes[i],LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_transform_pivot_x(eyes[i],12,0);lv_obj_set_style_transform_pivot_y(eyes[i],30,0);lv_obj_set_style_transform_rotation(eyes[i],50,0);
        smiles[i]=lv_arc_create(body);lv_obj_set_size(smiles[i],36,36);lv_arc_set_bg_angles(smiles[i],200,340);
        lv_obj_set_style_arc_color(smiles[i],lv_color_hex(C_EYE),LV_PART_MAIN);lv_obj_set_style_arc_width(smiles[i],10,LV_PART_MAIN);
        lv_obj_set_style_arc_rounded(smiles[i],true,LV_PART_MAIN);lv_obj_set_style_arc_opa(smiles[i],LV_OPA_TRANSP,LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(smiles[i],LV_OPA_TRANSP,LV_PART_KNOB);lv_obj_remove_flag(smiles[i],LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);lv_obj_add_flag(smiles[i],LV_OBJ_FLAG_HIDDEN);
    }
#endif
    caption=label(root,"",300);lv_obj_set_pos(caption,83,334);lv_obj_set_height(caption,38);lv_label_set_long_mode(caption,LV_LABEL_LONG_WRAP);
    voice_hint=label(root,PET_STR_HINT_CONNECTING,260);lv_obj_set_pos(voice_hint,103,391);
    lv_obj_remove_flag(voice_hint,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_color(voice_hint,lv_color_hex(C_HINT),0);
    handle=lv_button_create(root);lv_obj_remove_style_all(handle);lv_obj_set_size(handle,72,42);lv_obj_align(handle,LV_ALIGN_BOTTOM_MID,0,-8);
    lv_obj_t *bar=lv_obj_create(handle);lv_obj_remove_style_all(bar);lv_obj_set_size(bar,34,3);lv_obj_center(bar);
    lv_obj_set_style_bg_color(bar,lv_color_hex(C_BAR),0);lv_obj_set_style_bg_opa(bar,LV_OPA_COVER,0);lv_obj_set_style_radius(bar,RADIUS(2),0);lv_obj_remove_flag(bar,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(handle,open_drawer,LV_EVENT_CLICKED,NULL);
    drawer=overlay();lv_obj_t *title=label(drawer,PET_STR_SETTINGS,200);lv_obj_set_pos(title,133,63);lv_obj_set_style_text_font(title,&title_font,0);
    volume_slider=slider(drawer,PET_STR_VOLUME,102,0,100);brightness_slider=slider(drawer,PET_STR_BRIGHTNESS,164,10,100);
    quiet_switch=toggle(drawer,PET_STR_QUIET,225);captions_switch=toggle(drawer,PET_STR_CAPTIONS,265);speaker_switch=toggle(drawer,PET_STR_SPEAKER,305);
    lv_obj_t *connection=button(drawer,PET_STR_CONNECTION,170,connections);lv_obj_set_pos(connection,148,350);
    lv_obj_t *back=button(drawer,PET_STR_BACK,90,home);lv_obj_set_pos(back,188,402);
    connection_page=overlay();title=label(connection_page,PET_STR_CONNECTION,240);lv_obj_set_pos(title,113,58);lv_obj_set_style_text_font(title,&title_font,0);
    network_label=label(connection_page,"",290);lv_obj_set_pos(network_label,88,106);
    bridge_label=label(connection_page,"",290);lv_obj_set_pos(bridge_label,88,143);
    voice_label=label(connection_page,"",290);lv_obj_set_pos(voice_label,88,180);
    setup_label=label(connection_page,"",290);lv_obj_set_pos(setup_label,88,220);
    passkey_label=label(connection_page,"",250);lv_obj_set_pos(passkey_label,108,258);
    lv_obj_t *setup=button(connection_page,PET_STR_SETUP_BLE,230,provision);lv_obj_set_pos(setup,118,294);
    lv_obj_t *stop=button(connection_page,PET_STR_SETUP_CLOSE,200,close_provision);lv_obj_set_pos(stop,133,345);
    back=button(connection_page,PET_STR_BACK_HOME,120,home);lv_obj_set_pos(back,173,396);
    /* Línea pequeña en gris encima del título, igual en las dos páginas. */
    lv_obj_t *pages[2]={drawer,connection_page};
    for(int i=0;i<2;i++){
        header_labels[i]=label(pages[i],PET_DISPLAY_NAME,260);lv_obj_set_pos(header_labels[i],103,30);
        lv_obj_set_style_text_color(header_labels[i],lv_color_hex(C_HINT),0);
    }
    /* Un dibujo por refresco de pantalla (33 ms en la placa, CONFIG_LV_DEF_REFR_PERIOD). */
    lv_timer_create(draw,LV_DEF_REFR_PERIOD,NULL);
}
static void ui_task(void *unused){
    (void)unused;/* Montserrat 16 de LVGL solo trae ASCII: tildes, ñ, ¿ y ¡ salen de la
     * misma Montserrat (Latin-1). La fuente CJK de Unifont solo entra en el
     * build en chino (CONFIG_PET_LANG_ZH). */
#if PIXEL_THEME
    /* Unifont 16 px de 1 bit: ASCII y Latin-1 pixelados; en chino sigue la CJK, también Unifont. */
    font=pet_font_pixel_16;
#if defined(CONFIG_PET_LANG_ZH) && CONFIG_PET_LANG_ZH
    font.fallback=&muse_font_cjk_16;
#endif
    /* Títulos en Press Start 2P (16 px de ancho por letra: solo para textos cortos). */
    title_font=pet_font_title_16;title_font.fallback=&font;
    (void)latin;
#else
    latin=pet_font_latin_16;
#if defined(CONFIG_PET_LANG_ZH) && CONFIG_PET_LANG_ZH
    latin.fallback=&muse_font_cjk_16;
#endif
    font=lv_font_montserrat_16;font.fallback=&latin;title_font=font;
#endif
    lv_indev_t *touch_device=NULL;
    if(pet_board_display_start(&touch_device)&&pet_board()->display_lock(-1)){
        build();pet_board()->display_unlock();initialized=true;
    }
    xSemaphoreGive(ready_signal);vTaskDelete(NULL);
}
esp_err_t pet_ui_start(pet_ui_callback_t cb,void *context){
    if(view_lock)return ESP_ERR_INVALID_STATE;
    callback=cb;callback_context=context;view_lock=xSemaphoreCreateMutex();ready_signal=xSemaphoreCreateBinary();
    if(!view_lock||!ready_signal)return ESP_ERR_NO_MEM;
    view.pose=(pet_pose_t){.scale_x=1,.scale_y=1,.eye_open=1,.screen_on=true};view.brightness=80;view.battery=-1;
    if(xTaskCreatePinnedToCore(ui_task,"pet_ui_init",8192,NULL,5,NULL,1)!=pdPASS)return ESP_ERR_NO_MEM;
    if(xSemaphoreTake(ready_signal,pdMS_TO_TICKS(10000))!=pdTRUE)return ESP_ERR_TIMEOUT;
    return initialized?ESP_OK:ESP_FAIL;
}
void pet_ui_update(const pet_ui_view_t *next){if(view_lock){xSemaphoreTake(view_lock,portMAX_DELAY);view=*next;xSemaphoreGive(view_lock);}}
void pet_ui_home(void){pet_ui_bench_page(0);}
void pet_ui_bench_page(int next){if(view_lock&&next>=0&&next<=2){xSemaphoreTake(view_lock,portMAX_DELAY);requested_page=next;xSemaphoreGive(view_lock);}}
void pet_ui_request_snapshot(void){if(view_lock){xSemaphoreTake(view_lock,portMAX_DELAY);snapshot_requested=true;xSemaphoreGive(view_lock);}}
