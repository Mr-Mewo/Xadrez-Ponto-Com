#include <stdint.h>

const int INPUT_SIZE = 768;
const int HL_SIZE = 256;

const int SCALE = 400;
const int QA = 255;
const int QB = 64;

struct Network {
    int16_t accumulator_weights[INPUT_SIZE][HL_SIZE];
    int16_t accumulator_biases[HL_SIZE];
    int16_t output_weights[2 * HL_SIZE];
    int16_t output_bias;
};

struct Square
{
    
};


// Square: 0 - 63
// PieceType: Pawn = 0, Knight = 1, Bishop, Rook, Queen, King
// Side: White = 0, Black = 1

int calculateIndex(Square square, PieceType pieceType, Side side)
{
    return side * 64 * 6 + pieceType * 64 + square;
}