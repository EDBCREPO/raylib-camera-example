/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_CAMERA_CAMERA
#define NODEPP_CAMERA_CAMERA
#define onFrame function_t<void,frame_t>

#include <libuvc/libuvc.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { enum camera_frame_format {
    FORMAT_UNKNOWN = 0,  FORMAT_ANY = 0,
    FORMAT_UNCOMPRESSED, FORMAT_COMPRESSED,
    FORMAT_YUYV,         FORMAT_UYVY,
    FORMAT_RGB,          FORMAT_BGR,
    FORMAT_MJPEG,        FORMAT_H264,
    FORMAT_GRAY8,        FORMAT_GRAY16,
    FORMAT_BY8,          FORMAT_BA81,
    FORMAT_SGRBG8,       FORMAT_SGBRG8,
    FORMAT_SRGGB8,       FORMAT_SBGGR8,
    FORMAT_NV12,         FORMAT_P010,
    FORMAT_COUNT,
};}

namespace nodepp { class camera_t {
protected:

    struct frame_t {
        uint        count=0;
        int         height;
        int         width; 
        int         type;
        void*       data;
        ulong       size;
    };

    struct NODE {
        uvc_device_handle_t *devh = nullptr;
        uvc_context_t       * ctx = nullptr;
        uvc_device_t        * dev = nullptr;
        ptr_t<onFrame>      callback;
        string_t              err ;
        frame_t               frm ;
        ulong               stamp = 0;
        int                 state = 0;
    };  ptr_t<NODE> obj;

    static void frame_callback( uvc_frame_t *frame, void *user_data ) {

        if( user_data==nullptr || frame==nullptr ){ return; }

        auto    callback = type::cast<onFrame>( user_data );
        frame_t raw_frame; memset( &raw_frame, 0, sizeof( frame_t ) );

        raw_frame.type  = frame->frame_format; 
        raw_frame.size  = frame->data_bytes;
        raw_frame.height= frame->height;
        raw_frame.width = frame->width;
        raw_frame.data  = frame->data;
        raw_frame.count = 0;

        ( *callback )( raw_frame );

    }

    bool errno( uvc_error error ) const noexcept { switch( error ) {
        case UVC_ERROR_IO:             obj->err="Input/output error";      return 1; break;
        case UVC_ERROR_INVALID_PARAM:  obj->err="Invalid Parameter";       return 1; break;
        case UVC_ERROR_ACCESS:         obj->err="Access denied";           return 1; break;
        case UVC_ERROR_NO_DEVICE:      obj->err="No such device";          return 1; break;
        case UVC_ERROR_NOT_FOUND:      obj->err="Entity not found";        return 1; break;
        case UVC_ERROR_BUSY:           obj->err="Resource busy";           return 1; break;
        case UVC_ERROR_TIMEOUT:        obj->err="Operation timed out";     return 1; break;
        case UVC_ERROR_NO_MEM:         obj->err="Insufficient memory";     return 1; break;
        case UVC_ERROR_NOT_SUPPORTED:  obj->err="Operation not supported"; return 1; break;
        case UVC_ERROR_INVALID_DEVICE: obj->err="Device not supported";    return 1; break;
        case UVC_ERROR_INVALID_MODE:   obj->err="Mode not supported";      return 1; break;
        case UVC_ERROR_OTHER:          obj->err="something went wrong";    return 1; break;
        default:                       obj->err.clear();                   return 0; break;
    }                                  obj->err.clear();                   return 0; }

    enum uvc_frame_format get_type( uint type ) const noexcept { switch( type ) {
        case FORMAT_UNCOMPRESSED: return UVC_FRAME_FORMAT_UNCOMPRESSED; break;
        case FORMAT_COMPRESSED:   return UVC_FRAME_FORMAT_COMPRESSED;   break;
        case FORMAT_UNKNOWN:      return UVC_FRAME_FORMAT_UNKNOWN;      break;
        case FORMAT_SGRBG8:       return UVC_FRAME_FORMAT_SGRBG8;       break;
        case FORMAT_SGBRG8:       return UVC_FRAME_FORMAT_SGBRG8;       break;
        case FORMAT_SRGGB8:       return UVC_FRAME_FORMAT_SRGGB8;       break;
        case FORMAT_SBGGR8:       return UVC_FRAME_FORMAT_SBGGR8;       break;
        case FORMAT_GRAY16:       return UVC_FRAME_FORMAT_GRAY16;       break;
        case FORMAT_GRAY8:        return UVC_FRAME_FORMAT_GRAY8;        break;
        case FORMAT_MJPEG:        return UVC_FRAME_FORMAT_MJPEG;        break;
        case FORMAT_COUNT:        return UVC_FRAME_FORMAT_COUNT;        break;
        case FORMAT_YUYV:         return UVC_FRAME_FORMAT_YUYV;         break;
        case FORMAT_UYVY:         return UVC_FRAME_FORMAT_UYVY;         break;
        case FORMAT_H264:         return UVC_FRAME_FORMAT_H264;         break;
        case FORMAT_BA81:         return UVC_FRAME_FORMAT_BA81;         break;
        case FORMAT_NV12:         return UVC_FRAME_FORMAT_NV12;         break;
        case FORMAT_P010:         return UVC_FRAME_FORMAT_P010;         break;
        case FORMAT_RGB:          return UVC_FRAME_FORMAT_RGB;          break;
        case FORMAT_BGR:          return UVC_FRAME_FORMAT_BGR;          break;
        case FORMAT_BY8:          return UVC_FRAME_FORMAT_BY8;          break;
        default:                  return UVC_FRAME_FORMAT_ANY;          break; 
    }                             return UVC_FRAME_FORMAT_ANY; }

public:

    camera_t( uint16 vid, uint16 pid, const char* serial ) : obj( new NODE() ) {

        if( errno( uvc_init( &obj->ctx, nullptr ) ) )
          { process::error( obj->err ); return; }

        if( errno( uvc_find_device( obj->ctx, &obj->dev, vid, pid, serial ) ) ) 
          { process::error( obj->err ); return; }

        if( errno( uvc_open( obj->dev, &obj->devh ) ) )
          { process::error( obj->err ); return; }

        obj->state = 1; obj->stamp = process::now();
        
    }

   ~camera_t() noexcept { if( obj.count()>1 ){ return; } free(); }
    
    camera_t() : obj( new NODE() ) {}

    /*─······································································─*/
    
    bool is_closed() const noexcept { return !is_available(); }

    bool is_available() const noexcept { 
        return obj->state>=1 && obj->ctx!=nullptr && 
               process::now()-obj->stamp<3000; 
    }
    
    void close() const noexcept { free(); }

    /*─······································································─*/

    int get_produc_id() const noexcept {
        uvc_device_descriptor_t *descriptor;
        
        if( uvc_get_device_descriptor( obj->dev, &descriptor ) )
          { return -1; }  
          
        auto resp = descriptor->idProduct;
        uvc_free_device_descriptor(descriptor); return resp;
    };

    int get_vendor_id() const noexcept {
        uvc_device_descriptor_t *descriptor;
        
        if( uvc_get_device_descriptor( obj->dev, &descriptor ) )
          { return -1; }  
          
        auto resp = descriptor->idVendor;
        uvc_free_device_descriptor(descriptor); return resp;
    };
    
    /*─······································································─*/

    string_t get_device_product() const noexcept {
        uvc_device_descriptor_t *descriptor; char *desc_str = nullptr;
        
        if( uvc_get_device_descriptor( obj->dev, &descriptor ) )
          { return nullptr; }

        auto resp = string_t( descriptor->product, strlen( descriptor->product ) );
        uvc_free_device_descriptor(descriptor); return resp;
    };

    string_t get_device_manufacturer() const noexcept {
        uvc_device_descriptor_t *descriptor; char *desc_str = nullptr;
        
        if( uvc_get_device_descriptor( obj->dev, &descriptor ) )
          { return nullptr; }

        auto resp = string_t( descriptor->manufacturer, strlen( descriptor->manufacturer ) );
        uvc_free_device_descriptor(descriptor); return resp;
    };

    string_t get_device_serial() const noexcept {
        uvc_device_descriptor_t *descriptor; char *desc_str = nullptr;
        
        if( uvc_get_device_descriptor( obj->dev, &descriptor ) )
          { return nullptr; }

        auto resp = string_t( descriptor->serialNumber, strlen( descriptor->serialNumber ) );
        uvc_free_device_descriptor(descriptor); return resp;
    };
    
    /*─······································································─*/

    template < class... T >
    void start_recording( int type, int width, int height, int fps ) const {

        if( obj->state==2 || !is_available() ){ return; } 
            obj->state =2; auto self = type::bind( this );

            obj->callback = new onFrame([=]( frame_t frame ){
                memcpy( &self->obj->frm, &frame, sizeof(frame_t) );
                self->obj->stamp = process::now();
            }); uvc_stream_ctrl_t ctrl;

        if( errno( uvc_get_stream_ctrl_format_size( obj->devh, &ctrl, get_type(type), width, height, fps ) ) ) 
          { process::error( obj->err ); return; }

        if( errno( uvc_start_streaming( obj->devh, &ctrl, frame_callback, &obj->callback, 0 ) ) ) 
          { process::error( obj->err ); return; }

    }

    void stop_recording() const noexcept { if( is_available() ){ 
         uvc_stop_streaming( obj->devh ); obj->state = 1;
    }}

    frame_t* get_frame() const noexcept { 
        if( obj->frm.count > 1 ){ return nullptr; }
            obj->frm.count++; return &obj->frm; 
    }
    
    /*─······································································─*/

    void free() const noexcept { if( obj->state==0 ){ return; } obj->state=0;
        if( obj->devh!=nullptr ){ stop_recording(); uvc_close(obj->devh); }
        if( obj->dev !=nullptr ){ uvc_unref_device(obj->dev); }
        if( obj->ctx !=nullptr ){ uvc_exit(obj->ctx); }
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace camera {

    template< class... T >
    camera_t add( const T&... args ) { return camera_t(args...); }

    ptr_t<camera_t> scan() {
          uvc_context_t * ctx;
    try { uvc_device_t **devs; uint8 count = 0;

        if( uvc_init(&ctx, NULL)!=0 ){ throw ""; }

        if( uvc_get_device_list( ctx, &devs )!=0 )
          { throw ""; }

        while( devs[count]!=nullptr ){ count++; }
            
        ptr_t<camera_t> pointer ( count );

        for( int i=0; i<count; i++ ) {
             uvc_device_descriptor_t *desc;

            if( uvc_get_device_descriptor( devs[i], &desc )!=0 ) 
              { continue; } 
              
            pointer[count] = camera_t(
                desc->idVendor, desc->idProduct,
                desc->serialNumber
            );

            uvc_free_device_descriptor( desc );
        }   uvc_free_device_list( devs, count );

                   uvc_exit(ctx); return pointer;
    } catch(...) { uvc_exit(ctx); return nullptr; } }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#undef onFrame
#endif