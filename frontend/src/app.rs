use std::cmp::{max, min};
use web_sys::{
    MouseEvent,
    // HtmlElement
    // window,
    // Element,
};
use yew::prelude::*;

use crate::{
    custom_tags as tags,
    game,
};

#[function_component(App)]
pub fn app() -> Html {
    let px = use_state(|| 4 );

    let px_add = {
        let px = px.clone();
        Callback::from(move |_: MouseEvent| px.set(min(*px + 1, 5)))
    };
    let px_sub = {
        let px = px.clone();
        Callback::from(move |_: MouseEvent| px.set(max(*px - 1, 2)))
    };

    // Ensures that the game is started when the app is loaded
    // use_effect(|| {
    //     game::game_start();
    //
    //     //TODO: Connects to the server here right?
    //     // - Might do this inside game_start() tho, sounds more natural
    // });

    html! {
        <main>
            <style>
                { format!(":root{} --px: {} {}", "{", *px, "}") }
            </style>
         
            <div class="piece-parent">
                <game::PieceParent/>
            </div>

            <tags::NineSlice src="9sl/tl-purple-big.png" class="main" size={*px*19}>
                <div class="header" data-tauri-drag-region="true">
                    <div class="title">
                        // {if !cfg!(feature = "tauri") { "XadrezPontoCom.com" } else { "XadrezPontoCom.exe" }}
                        <img src={if !cfg!(feature = "tauri") { "assets/img/xadrez-header-com.png" } else { "assets/img/xadrez-header-exe.png" }} />
                    </div>

                    //TODO:
                    // - Refactor to use a single 'HeaderButtons' component
                    // - Make smaller buttons because it's uuuuglyyyy :(
                    <div class="header-buttons">
                        <button onclick={px_sub} class="minus-btn">
                            <img src="assets/img/header-buttons/minus.png"/>
                        </button>
                        <button onclick={px_add} class="plus-btn">
                            <img src="assets/img/header-buttons/plus.png"/>
                        </button>

                        if cfg!(feature = "tauri") {
                            <tags::TauriWindowButtons />
                        }
                    </div>
                </div>

                <div class="content">
                    // Possibly unclear, but that's the ENTIRE chessboard

                    <game::ChessBoard
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
