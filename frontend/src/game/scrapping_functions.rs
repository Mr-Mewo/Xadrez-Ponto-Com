use wasm_bindgen::JsCast;
use web_sys::{Element, HtmlElement};
use crate::game::piece::Pos;

macro_rules! get_doc { () => {{ web_sys::window().unwrap().document().unwrap() }} }

pub fn get_square_pos_by_id(sqr_id: &str) -> Pos {
    let square = get_doc!().query_selector(&format!("#{}", sqr_id)).unwrap().unwrap();
    let rect = square.get_bounding_client_rect();

    Pos::new(
        (rect.left() + rect.width() / 2.0) as i32,
        (rect.top() + rect.height() / 2.0) as i32,
    )
}

pub fn get_any_square_size() -> f64 {
    let square = get_doc!().query_selector(".square").unwrap().unwrap();
    square.get_bounding_client_rect().height()
}

pub fn get_class_at_position(class: &str, x: i32, y: i32) -> Option<Element> {
    get_doc!()
        .elements_from_point(x as f32, y as f32)
        .iter()
        .find_map(|el| el.dyn_ref::<Element>().filter(|e| e.class_name().contains(class)).cloned())
}

pub fn get_piece_at_square(id: &str) -> Option<HtmlElement> {
    let element = get_doc!().get_element_by_id(&format!("p-{}", id))?;
    element.dyn_into::<HtmlElement>().ok()
}

pub fn get_element_by_id(id: &str) -> Option<Element> {
    get_doc!().get_element_by_id(id)
}
