use yew::prelude::*;
#[allow(unused_imports)] use wasm_bindgen::{prelude::*, JsCast};

use crate::custom_tags as ct;

#[function_component(App)]
pub fn app() -> Html {
    let px = use_state(|| 4);

    let _px_add = {
        let px = px.clone();
        Callback::from(move |_: i32| px.set(*px + 1))
    };
    let _px_sub = {
        let px = px.clone();
        Callback::from(move |_: i32| px.set(*px - 1))
    };

    let mut _input = String::new();
    let _letters = String::from("abcdefgh");

    html! {
        <main>
            <style>
                { format!(":root{} --px: {} {}", "{", *px, "}") } // ref as to avoid a borrow
            </style>

            // <button onclick={ _px_sub }>{ "-" }</button>
            // <button onclick={ _px_add }>{ "+" }</button>

            <ct::NineSlice src="assets/img/tl-blue.png" size={*px*12}>

                <div class="header">
                    <div class="title">
                        {"XadrezPontoCom.com"}
                    </div>
                    <div class="tauri-buttons">
                        {"XadrezPontoCom.com"}
                    </div>
                </div>

                <div class="content">
                    <div class="board">

                    </div>
                    <div class="side-panel">

                    </div>
                </div>

                <div class="footer"></div>
            </ct::NineSlice>
        </main>
    }
}
