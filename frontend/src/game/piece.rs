use chess::MoveGen;
use wasm_bindgen::JsCast;
use web_sys::MouseEvent;
use yew::{
    html,
    Html,
    classes,
    function_component,
    use_effect_with,
    use_state,
    Callback,
    Properties,
    UseStateHandle
};
use crate::{
    game::{register_piece, game_functions::*},
    log, get_square_position, get_square_size, get_legal_moves
};

#[derive(PartialEq, Clone, Debug)]
pub struct Pos { x: i32, y: i32 }

impl Pos { pub(crate) fn new(x: i32, y: i32) -> Self {
     Self { x, y }
} }


#[derive(Properties, PartialEq, Clone)]
pub struct PieceProps {
    pub sqr_id: String,
    pub piece_char: char,

    // Naturally, it starts without a position
    pub prp_sqr_id: UseStateHandle< String >,
    pub prp_pos: UseStateHandle< Pos >
}

#[function_component(Piece)]
pub fn piece(props: &PieceProps) -> Html {
    let mounted = use_state(|| false);

    let dragging = use_state(|| false);
    let offset = use_state(|| Pos::new(0, 0));
    let board = use_state( get_board );
    let square_size = use_state(|| 0f64);

    use_effect_with(props.sqr_id.clone(), {
        let mounted = mounted.clone();
        let prp_sqr_id = props.prp_sqr_id.clone();
        let prp_pos = props.prp_pos.clone();
        let square_size = square_size.clone();

        let props = props.clone();

        move |init_sqr_id| {
            register_piece(&props);
            // if *mounted { return; }
            log!("Initializing piece at {}", init_sqr_id);
            prp_sqr_id.set(init_sqr_id.clone());
            prp_pos.set(get_square_position!(init_sqr_id.as_str()));
            mounted.set(true);
            square_size.set(get_square_size!());
        }
    });

    let curr_legal = use_state(|| get_legal_moves!(&board, &*props.prp_sqr_id));

    /* ------------------------------------------------------------------------------------------ */

    let on_drag_start = Callback::from({
        let sqr_id = props.prp_sqr_id.clone();
        let pos = props.prp_pos.clone();

        let dragging = dragging.clone();
        let offset = offset.clone();

        let board = board.clone();
        let curr_legal = curr_legal.clone();

        let mounted = *mounted;


        move |e: MouseEvent| {
            if !mounted { return; }
            log!("on_drag_start {}", *sqr_id);

            e.prevent_default(); // Prevents default browser click behavior

            dragging.set(true);
            offset.set(Pos::new(e.client_x() - pos.x, e.client_y() - pos.y));

            let aux_board = get_board();
            board.set(aux_board);

            curr_legal.set(get_legal_moves!(&aux_board, &*sqr_id));
        }
    });

    /* ------------------------------------------------------------------------------------------ */

    let while_dragging = Callback::from({
        let sqr_id = props.prp_sqr_id.clone();
        let pos = props.prp_pos.clone();

        let dragging = dragging.clone();

        let mounted = *mounted;


        move |e: MouseEvent| {
            if !mounted || !*dragging { return }
            log!("while_dragging {}", *sqr_id);

            pos.set(Pos::new(e.client_x() - offset.x, e.client_y() - offset.y));
        }
    });

    /* ------------------------------------------------------------------------------------------ */

    let on_drag_end = Callback::from({
        let sqr_id = props.prp_sqr_id.clone();
        let pos = props.prp_pos.clone();

        let dragging = dragging.clone();

        let pos = pos.clone();
        let mounted = *mounted;


        move |e: MouseEvent| {
            e.prevent_default(); // Prevents default browser click behavior
            if !mounted { return; }
            log!("on_drag_end {}", *sqr_id);

            dragging.set(false);

            if let Some(element) = get_class_at_position("square", pos.x, pos.y) {
                let new_id = element.id();
                let mv = format!("{}{}", *sqr_id, new_id);

                match move_it(mv.as_str()) {
                    Ok(_) => {
                        sqr_id.set(new_id.to_string());
                        pos.set(get_square_position!(new_id.as_str()));

                        // If it overlaps with another piece
                        if let Some(eaten_piece) = get_piece_at_square( new_id.as_str() ) {
                            // Places the "eaten" class on the eaten piece
                            eaten_piece.set_class_name(format!("{} eaten", eaten_piece.class_name()).as_str());
                            eaten_piece.set_id("");
                        }
                    },
                    Err(e) => {
                        // Invalid move. Return to the original position
                        log!("Invalid move: {}", e);

                        pos.set(get_square_position!(sqr_id.as_str()));
                    }
                }
            }else{
                // Outside board bounds. Return to the original position

                pos.set(get_square_position!(sqr_id.as_str()));
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
}
