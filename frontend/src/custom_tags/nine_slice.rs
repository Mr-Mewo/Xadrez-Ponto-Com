use yew::prelude::*;
use crate::log;

#[derive(Properties, PartialEq)]
pub struct NineSliceProps {
    pub src_begin: String,
    #[prop_or(".png".to_string())]
    pub src_end: String,
    #[prop_or(48)]
    pub size: u32,
    // Inner tags
    #[prop_or_default]
    pub children: Children,
}

#[function_component(NineSlice)]
pub fn nine_slice(props: &NineSliceProps) -> Html {
    let src_b = format!("assets/img/{}", props.src_begin.clone());
    let src_e = props.src_end.clone();
    let size = props.size.clone();

    let hr= vec!["left", "center", "right"];
    let vr= vec!["top", "center", "down"];

    html! {
        // "--9slc-size" is an optional variable usable in CSS
        <div class="nine-slice" style={format!("--9sz: {size}px;")}>
        
        // Render Inner tags
        { for props.children.iter() }
        
        <div class="slices"> {
            hr.iter().enumerate().map(|(i, _)| {
                vr.iter().enumerate().map(|(j, _)| {
                    // let src = format!("{}{}{}{}", src_b, j, i, src_e);
                    let src = format!("{}{}", src_b, src_e);

                    log!("src: {}", src);

                    let style = format!(
                        "position: absolute;
                        width: {};
                        height: {};
                        /*border: 1px solid red;*/
                        background-image: url({src});
                        background-size: {} {};
                        background-position: {} {};
                        background-repeat: no-repeat;
                        content: '{i}{j}';
                        left: {};
                        top: {};",
                        if i == 1 { format!("calc(100% - {size}*2px)") } else { size.to_string()+ "px" },
                        if j == 1 { format!("calc(100% - {size}*2px)") } else { size.to_string()+ "px" },
                        if i == 1 { "1000%".to_string() } else { (size*3).to_string()+ "px" },
                        if j == 1 { "1000%".to_string() } else { (size*3).to_string()+ "px" },
                        if i == 1 { "50%".to_string() } else { (-(i as i32 * size as i32)).to_string()+ "px" },
                        if j == 1 { "50%".to_string() } else { (-(j as i32 * size as i32)).to_string()+ "px" },
                        if i == 1 { size.to_string()+ "px" } else { format!("calc((50% - {size}px/2)*{i})") },
                        if j == 1 { size.to_string()+ "px" } else { format!("calc((50% - {size}px/2)*{j})") },
                    );
                
                    /*
                    let style = format!(
                        "position: absolute;
                        width: {};
                        height: {};
                        border: 1px solid red;
                        background-image: url({src});
                        background-size: {} {};
                        background-position: {} {};
                        background-repeat: no-repeat;
                        content: '{i}{j}';
                        left: {};
                        top: {};",
                        if i == 1 { format!("calc(100% - {size}*2px)") } else { size.to_string()+ "px" },
                        if j == 1 { format!("calc(100% - {size}*2px)") } else { size.to_string()+ "px" },
                        if i == 1 { format!("calc(100% + {size}*6px)") } else { (size*3).to_string()+ "px" },
                        if j == 1 { format!("calc(100% + {size}*6px)") } else { (size*3).to_string()+ "px" },
                        if i == 1 { format!("calc({size}*-3px)") } else { (-(i as i32 * size as i32)).to_string()+ "px" },
                        if j == 1 { format!("calc({size}*-3px)") } else { (-(j as i32 * size as i32)).to_string()+ "px" },
                        // -(i as i32 * size as i32),  // Horizontal position
                        // -(j as i32 * size as i32),  // Vertical position
                        if i == 1 { size.to_string()+ "px" } else { format!("calc((50% - {size}px/2)*{i})") },
                        if j == 1 { size.to_string()+ "px" } else { format!("calc((50% - {size}px/2)*{j})") },
                    );
                    */
                    
                    html! {
                        <div class="slice" style={ style }></div>
                    }
                }).collect::<Html>()
            }).collect::<Html>()
        } </div/* slices */></div/* nine-slice */>
    }
}