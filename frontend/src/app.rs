use yew::prelude::*;

#[function_component(App)]
pub fn app() -> Html {
    html! {
        <main>
            <div class="board"></div>

            <div class="side-panel">
                <div class="panel-1"></div>
                <div class="panel-2"></div>
            </div>
        </main>
    }
}
