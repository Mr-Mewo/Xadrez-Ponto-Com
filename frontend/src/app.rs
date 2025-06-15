use yew::prelude::*;
use crate::{
    *,
    custom_tags as ct
};

#[function_component(App)]
pub fn app() -> Html {
    let px = use_state(|| 3 );
    let onmousemove = get_mouse_callback();

    let _px_add = {
        let px = px.clone();
        Callback::from(move |_: i32| px.set(*px + 1))
    };
    let _px_sub = {
        let px = px.clone();
        Callback::from(move |_: i32| px.set(*px - 1))
    };
    
    let tauri = cfg!(feature = "tauri");

    html! {
        <main onmousemove={onmousemove}>
            <style>
                { format!(":root{} --px: {} {}", "{", *px, "}") }
            </style>

            <ct::NineSlice src="9sl/tl-purple.png" size={*px*12}>
                <div class="header" data-tauri-drag-region="true">
                    <div class="title">
                        {if !tauri { "XadrezPontoCom.com" } else { "XadrezPontoCom.exe" }}
                    </div>

                    // Window buttons, only when in tauri
                    // Could have extra here while on Web
                    // todo: <MORE/>
                    <ct::TauriWindowButtons />
                </div>

                <div class="content">
                    // Possibly unclear, but that's the ENTIRE chessboard
                    
                    <ct::ChessBoard 
                        pixel_size={*px} 
                        white_src="white-2.png" 
                        black_src="black-1.png"
                    />

                    <div class="side-panel">
                        
                    </div>
                </div>

                // Could turn into padding-bottom, there's nothing inside it, ┐(´～｀;)┌ 
                // but...: "Don't change what ain't broken" - Some internet smartass
                // Why am I ranting about padding?
                // It's 2AM I should sleep
                <div class="footer"></div>
            </ct::NineSlice>
        </main>
    }
}