use yew::prelude::*;

use crate::{
    custom_tags as tags,
    game,
};

#[function_component(App)]
pub fn app() -> Html {
    let px = use_state(|| 4 );

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

                    <tags::HeaderButtons px_ref={px.clone()}/>
                </div>

                <div class="content">
                    // Possibly unclear, but that's the ENTIRE chessboard

                    <tags::ChessBoard
                        pixel_size={*px}
                        white_src="white-2.png"
                        black_src="black-1.png"
                    />

                    <tags::NineSlice src="9sl/tl-green.png" class="side-panel" size={*px*12} />
                </div>

                // Could turn into padding-bottom, there's nothing inside it, ┐(´～｀;)┌
                // but...: "Don't change what ain't broken" - Some internet smartass
                // Why am I ranting about padding?
                // It's 2AM I should sleep
                <div class="footer" />
            </tags::NineSlice>
        </main>
    }
}
