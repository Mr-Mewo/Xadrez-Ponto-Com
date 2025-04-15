use yew::prelude::*;

#[function_component(App)]
pub fn app() -> Html {
    html! {
        <main>
            <NineSlice src="assets/img/tile-window.png" size=36/>
            <div class="board"></div>
            <p class="title">{"Yew Game"}</p>
            <div class="side-panel">
                <div class="panel-1"></div>
                <div class="panel-2"></div>
            </div>
        </main>
    }
}

#[derive(Properties, PartialEq)]
pub struct NineSliceProps {
    pub src: String,
    #[prop_or(36)]
    pub size: u32,
}

#[function_component(NineSlice)]
pub fn nine_slice(props: &NineSliceProps) -> Html {
    let src = props.src.clone();
    let size = props.size.clone();

    let style = format!(
        "--img: url('{}'); --size: {}px;",
        props.src, props.size/3
    );

    html! {
        <div class="nine-slice" style={style}>
            <p>
                // {props.src.clone()}
            </p>
            <div class="top-left"></div>
            <div class="top"></div>
            <div class="top-right"></div>
            <div class="left"></div>
            <div class="center"></div>
            <div class="right"></div>
            <div class="bottom-left"></div>
            <div class="bottom"></div>
            <div class="bottom-right"></div>
        </div>
    }
}