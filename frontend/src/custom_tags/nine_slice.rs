use yew::prelude::*;

#[derive(Properties, PartialEq)]
pub struct NineSliceProps {
    #[prop_or_default]
    pub children: Children,
    #[prop_or_default]
    pub class: Classes,
    #[prop_or_default]
    pub id: String,

    pub src: String,
    #[prop_or(48)]
    pub size: u32,
}

// I'm not going to touch this thing EVER AGAIN!
#[function_component(NineSlice)]
pub fn nine_slice(props: &NineSliceProps) -> Html {
    let src = format!("assets/img/{}", &props.src);
    let size = props.size;

    let mut slices = Vec::new();

    for h in 0..3 {
        for v in 0..3 {
            let width = if h == 1 { format!("calc(100% - {size}*2px)") } else { format!("{size}px") };
            let height = if v == 1 { format!("calc(100% - {size}*2px)") } else { format!("{size}px") };
            
            let bg_sz_x = if h == 1 { "1000%".to_string() } else { format!("{}px", size*3) };
            let bg_sz_y = if v == 1 { "1000%".to_string() } else { format!("{}px", size*3) };
            
            let bg_pos_x = if h == 1 { "50%".to_string() } else { format!("{}px", -(h * size as i32)) };
            let bg_pos_y = if v == 1 { "50%".to_string() } else { format!("{}px", -(v * size as i32)) };
            
            let left = if h == 1 { format!("{size}px") } else { format!("calc((50% - {size}px/2)*{h})") };
            let top = if v == 1 { format!("{size}px") } else { format!("calc((50% - {size}px/2)*{v})") };
            
            let style = format!(
                "position: absolute;
                width: {width};
                height: {height};
                background-image: url({src});
                background-size: {bg_sz_x} {bg_sz_y};
                background-position: {bg_pos_x} {bg_pos_y};
                background-repeat: no-repeat;
                left: {left};
                top: {top};"
            );
            
            slices.push(html! {
                <div class="slice" style={ style }></div>
            });
        }
    }

    html! {
        <div class={classes!("nine-slice", props.class.clone())} style={format!("--9sz: {size}px;")} id={props.id.clone()}>
            { for props.children.iter() }
            <div class="slices">
                { for slices.into_iter() }
            </div>
        </div>
    }
}