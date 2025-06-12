use yew::prelude::*;
use crate::{
    *,
    custom_tags as ct
};

#[function_component(App)]
pub fn app() -> Html {
    let px = use_state(|| 3);
    let onmousemove = get_mouse_callback();

    let _px_add = {
        let px = px.clone();
        Callback::from(move |_: i32| px.set(*px + 1))
    };
    let _px_sub = {
        let px = px.clone();
        Callback::from(move |_: i32| px.set(*px - 1))
    };
    
    html! {
        <main onmousemove={onmousemove}>
            <style>
                { format!(":root{} --px: {} {}", "{", *px, "}") }
            </style>

            <ct::NineSlice src="9sl/tl-green.png" size={*px*12}>
                <div class="header" data-tauri-drag-region="true">
                    <div class="title">
                        {"XadrezPontoCom.com"}
                    </div>

                    // Window buttons, only when in tauri
                    // Could have extra here while on Web
                    // todo: <MORE/>
                    <ct::TauriWindowButtons />
                </div>

                <div class="content">
                    // Could be unclear, but that's the ENTIRE chessboard
                    <ct::ChessBoard 
                        pixel_size={*px} 
                        white_src="white-2.png" 
                        black_src="black-1.png"
                    />

                    <div class="side-panel">
                        
                    </div>
                </div>

                // With the current setup, this is just for padding
                // Could turn into padding-bottom ┐(´～｀;)┌
                <div class="footer"></div>
            </ct::NineSlice>
        </main>
    }
}