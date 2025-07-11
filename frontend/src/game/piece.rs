use web_sys::MouseEvent;
use yew::{
    classes,
    function_component,
    html,
    use_effect_with,
    use_state,
    Callback,
    Html,
    Properties,
    UseStateHandle
};
use crate::{
    game::{
        game_functions::*,
        scrapping_functions::*
    },
    util::piece_src,
};
use crate::game::websocket::{is_connected,
                             send_message
};
#[allow(unused_imports)]
use crate::log;

#[derive(PartialEq, Clone, Debug)]
pub struct Pos { x: i32, y: i32 }

impl Pos { pub(crate) fn new(x: i32, y: i32) -> Self {
     Self { x, y }
} }


#[derive(Properties, PartialEq, Clone)]
pub struct PieceProps {
    pub sqr_id: String,
    pub piece_char: char,

    pub prp_sqr_id: UseStateHandle< String >,
    pub prp_pos: UseStateHandle< Pos >
}

#[function_component(Piece)]
pub fn piece(props: &PieceProps) -> Html {
    let dragging = use_state(|| false);
    let offset = use_state(|| Pos::new(0, 0));
    let board = use_state( get_board );
    let square_size = use_state(|| 0f64);

    let curr_legal = use_state(Vec::new );

    use_effect_with(props.sqr_id.clone(), {
        let prp_sqr_id = props.prp_sqr_id.clone();
        let prp_pos = props.prp_pos.clone();
        let square_size = square_size.clone();

        move |init_sqr_id| {
            // if *mounted { return; }
            // log!("Initializing piece at {}", init_sqr_id);
            prp_sqr_id.set(init_sqr_id.clone());
            prp_pos.set(get_square_pos_by_id(init_sqr_id.as_str()));
            square_size.set(get_any_square_size());
        }
    });

    /* ------------------------------------------------------------------------------------------ */

    let on_drag_start = Callback::from({
        let sqr_id = props.prp_sqr_id.clone();
        let pos = props.prp_pos.clone();

        let dragging = dragging.clone();
        let offset = offset.clone();

        let board = board.clone();
        let curr_legal = curr_legal.clone();


        move |e: MouseEvent| {
            e.prevent_default(); // Prevents default browser click behavior

            dragging.set(true);
            offset.set(Pos::new(e.client_x() - pos.x, e.client_y() - pos.y));

            let aux_board = get_board();
            board.set(aux_board);
            let legal = get_legal_moves(&aux_board, &sqr_id);

            for move_sqr in &legal {
                let selected_square = get_element_by_id(move_sqr).unwrap().query_selector(".movable-to").unwrap().unwrap();
                selected_square.set_class_name(format!("{} visible", selected_square.class_name()).as_str());
            }

            curr_legal.set(legal);
        }
    });

    /* ------------------------------------------------------------------------------------------ */

    let while_dragging = Callback::from({
        let pos = props.prp_pos.clone();

        let dragging = dragging.clone();


        move |e: MouseEvent| {
            if !*dragging { return }

            pos.set(Pos::new(e.client_x() - offset.x, e.client_y() - offset.y));
        }
    });

    /* ------------------------------------------------------------------------------------------ */

    let on_drag_end = Callback::from({
        let props = props.clone();

        let sqr_id = props.prp_sqr_id.clone();
        let pos = props.prp_pos.clone();

        let dragging = dragging.clone();

        let pos = pos.clone();
        ;
        let curr_legal = curr_legal.clone();


        move |e: MouseEvent| {
            e.prevent_default(); // Prevents default browser click behavior

            dragging.set(false);

            for move_sqr in &*curr_legal {
                let selected_square = get_element_by_id(move_sqr).unwrap().query_selector(".movable-to").unwrap().unwrap();
                selected_square.set_class_name(selected_square.class_name().replace(" visible", "").as_str());
            }

            if let Some(element) = get_class_at_position("square", pos.x, pos.y) {
                let new_id = element.id();
                let mv = format!("{}{}", *sqr_id, new_id);

                match make_move(mv.as_str(), Some(&props.clone())) {
                    Ok(_) => {
                        if is_connected() && props.piece_char.is_ascii_uppercase() {
                            // Valid
                            // this part used to be more useful
                            // Don't cry :(

                            let move_data = format!(r#"{{"type":"move","move":"{}","fen":"{}","depth":9}}"#,
                            mv, get_board());

                            send_message(&move_data);
                        }
                    },
                    Err(_e) => {
                        // Invalid move. Return to the original position

                        pos.set(get_square_pos_by_id(sqr_id.as_str()));
                    }
                }
            }else{
                // Outside bounds. Return to the original position

                pos.set(get_square_pos_by_id(sqr_id.as_str()));
            }
        }
    });

    /* ------------------------------------------------------------------------------------------ */

    let style = format!("position: absolute; left: {}px; top: {}px; height: {}px; {}",
                        props.prp_pos.x - (*square_size/2.0) as i32, props.prp_pos.y - (*square_size/2.0) as i32, *square_size,
                        if *dragging { "z-index: 6; pointer-events: none" }
                        else { "cursor: grab; transition: all 0.2s ease-out" }
    );

    /* ------------------------------------------------------------------------------------------ */

    if true { //props.piece_char.is_ascii_uppercase() {
        html! {
            // Funny thing: Hovering off the piece will make it unable to activate the Callbacks
            // The solution is to transfer the mouse events to a div that covers the entire screen
            // It'll ensure the events are always captured

            <>
                <div
                    {style} class={classes!("piece", props.piece_char.to_string(), if *dragging { "dragging" } else { "" })}
                    id = {format!("p-{}", *props.prp_sqr_id)}
                    onmousedown = {on_drag_start}
                >
                    <img
                        src={ piece_src(props.piece_char) }
                        draggable="false"
                    />
                </div>
                if *dragging {
                    <div style="position: absolute; left: 0; top: 0; width: 100%; height: 100%; z-index: 5; cursor: grabbing;"
                        onmousemove = {while_dragging}
                        onmouseup = {on_drag_end.clone()}
                        onmouseleave = {on_drag_end}
                    />
                }
            </>
        }
    }else{
        html! {
            <div
                {style} class={classes!("piece", props.piece_char.to_string(), if *dragging { "dragging" } else { "" })}
                    id = {format!("p-{}", *props.prp_sqr_id)}
            >
                <img
                    src={ piece_src(props.piece_char) }
                    draggable="false"
                />
            </div>
        }
    }
}
