#include <nodepp/nodepp.h>
#include <camera/camera.h>
#include <raylib/game.h>

using namespace nodepp;

void onMain() {

    ptr_t<rl::Texture> txt = new rl::Texture();
    auto cam = camera::scan()[0]; 

    cam.start_recording( FORMAT_MJPEG, 640, 480, 30 );
    rl::Init( 640, 480, 60, "raylib camera" );

    ptr_t<rl::Shader> shaders({
        rl::LoadShader( 0, "./shader/shader0.fs" ),
        rl::LoadShader( 0, "./shader/shader1.fs" ),
        rl::LoadShader( 0, "./shader/shader2.fs" ),
        rl::LoadShader( 0, "./shader/shader3.fs" ),
        rl::LoadShader( 0, "./shader/shader4.fs" )
    });

    rl::onLoop([=]( float delta ){ ([=](){
    coStart

        while( cam.is_available() ){ do {
            auto frame = cam.get_frame(); if( frame==nullptr ){ break; } rl::UnloadTexture( *txt ); 
            auto tmp = rl::LoadImageFromMemory( ".jpeg", (uchar*) frame->data, frame->size );
            auto pmt = rl::LoadTextureFromImage( tmp ); rl::UnloadImage( tmp );
            memcpy( &txt, &pmt, sizeof( rl::Texture ) );
        } while(0); coNext; }

    coStop
    })(); });

    rl::onDraw([=](){

        if( rl::IsTextureValid( *txt ) ){
            rl::ClearBackground( rl::BLACK );
            rl::BeginShaderMode( shaders[1] );
                rl::DrawTexture( *txt, 0, 0, rl::WHITE );
            rl::EndShaderMode();
        }   rl::DrawFPS( 10, 10 );

    });

    rl::onClose([=](){
        forEach( x, shaders ){ rl::UnloadShader(x); }
        console::log("Closed"); cam.stop_recording();
        process::exit(1);
    });

}