use std::cmp::{max, min};
use web_sys::MouseEvent;
use yew::prelude::*;
use crate::{
    custom_tags as tags,
    game::ChessBoard,
    util::get_mouse_callback,
};

#[function_component(App)]
pub fn app() -> Html {
    let px = use_state(|| 3 );
    let onmousemove = get_mouse_callback();

    let _px_add = {
        let px = px.clone();
        Callback::from(move |_: MouseEvent| px.set(min(*px + 1, 5)))
    };
    let _px_sub = {
        let px = px.clone();
        Callback::from(move |_: MouseEvent| px.set(max(*px - 1, 2)))
    };

    html! {
        <main onmousemove={onmousemove}>
            <style>
                { format!(":root{} --px: {} {}", "{", *px, "}") }
            </style>

            <tags::NineSlice src="9sl/tl-purple.png" class="main" size={*px*12}>
                <div class="header" data-tauri-drag-region="true">
                    <div class="title">
                        {if !cfg!(feature = "tauri") { "XadrezPontoCom.com" } else { "XadrezPontoCom.exe" }}
                    </div>

                    // Window buttons, only when in tauri
                    // Could have extra here while on Web
                    // todo: <MORE/>
                    <div class="header-buttons">

                        <button onclick={_px_sub} class="minus">{"-"}</button>
                        <button onclick={_px_add} class="plus">{"+"}</button>

                    if cfg!(feature = "tauri") {
                        <tags::TauriWindowButtons />
                    }
                    </div>
                </div>

                <div class="content">
                    // Possibly unclear, but that's the ENTIRE chessboard
                    
                    <ChessBoard 
                        pixel_size={*px} 
                        white_src="white-2.png" 
                        black_src="black-1.png"
                    />

                    <tags::NineSlice src="9sl/tl-green.png" class="side-panel" size={*px*12}>
                        
                    </tags::NineSlice>
                </div>

                // Could turn into padding-bottom, there's nothing inside it, ┐(´～｀;)┌ 
                // but...: "Don't change what ain't broken" - Some internet smartass
                // Why am I ranting about padding?
                // It's 2AM I should sleep
                <div class="footer"></div>
            </tags::NineSlice>
        </main>
    }
}