#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <unistd.h>
#include <stdlib.h>

    #define U64 unsigned long long
    U64 bitboard;

    // FEN dedug positions
        #define empty_board "8/8/8/8/8/8/8/8 w - - "
        #define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
        #define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
        #define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
        #define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "
        #define repetitions "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40 "

//////////////////////////////////////////////////////////// Board //////////////////////////////////////////////////////////////////////////////////////
 
    // set/get/pop macros
        #define set_bit(bitboard, square) (bitboard |= (1ULL << square))
        #define get_bit(bitboard, square) (bitboard & (1ULL << square))
        #define pop_bit(bitboard, square) (get_bit(bitboard, square) ? bitboard ^= (1ULL << square) : 0)

    // print bitboard
        void print_bitboard(U64 bitboard){

            printf("\n");

            // loop over board ranks
            for (int rank = 0; rank < 8; rank++){

                // loop over board files
                for (int file = 0; file < 8; file++){

                    // convert file & rank into square index
                    int square = rank * 8 + file;
                    
                    // print ranks
                    if (!file)
                        printf("  %d ", 8 - rank);
                    
                    // print bit state (either 1 or 0)
                    printf(" %d", get_bit(bitboard, square) ? 1 : 0);
                    
                }
                
                // print new line every rank
                printf("\n");
            }
            
            // print board files
            printf("\n     a b c d e f g h\n\n");
            
            // print bitboard as unsigned decimal number
            printf("     Bitboard: %llud\n\n", bitboard);
        }

    // side to move
        enum { white, black, both };

    // bishop and rook
        enum { rook, bishop };

    // squares
        enum{
            a8, b8, c8, d8, e8, f8, g8, h8,         // 56 .. 63
            a7, b7, c7, d7, e7, f7, g7, h7,         // 48 .. 55
            a6, b6, c6, d6, e6, f6, g6, h6,         // 40 .. 47
            a5, b5, c5, d5, e5, f5, g5, h5,         // 32 .. 39
            a4, b4, c4, d4, e4, f4, g4, h4,         // 24 .. 31
            a3, b3, c3, d3, e3, f3, g3, h3,         // 16 .. 23
            a2, b2, c2, d2, e2, f2, g2, h2,         //  8 .. 15
            a1, b1, c1, d1, e1, f1, g1, h1, no_sq,  //  0 ..  7
        };

    // encode pieces
        enum { P, N, B, R, Q, K, p, n, b, r, q, k };

    /*

            bin  dec
                
        0001    1  white king can castle to the king side
        0010    2  white king can castle to the queen side
        0100    4  black king can castle to the king side
        1000    8  black king can castle to the queen side

        examples

        1111       both sides an castle both directions
        1001       black king => queen side
                    white king => king side

    */

    enum { wk = 1, wq = 2, bk = 4, bq = 8 };

    // ASCII pieces
        char ascii_pieces[12] = "PNBRQKpnbrqk";

        const char *square_to_coordinates[] = {
            "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
            "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
            "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
            "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
            "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
            "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
            "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
            "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        };

    // convert ASCII character pieces to encoded constants
        int char_pieces[] = {
            ['P'] = P,
            ['N'] = N,
            ['B'] = B,
            ['R'] = R,
            ['Q'] = Q,
            ['K'] = K,
            ['p'] = p,
            ['n'] = n,
            ['b'] = b,
            ['r'] = r,
            ['q'] = q,
            ['k'] = k
        };

    // promoted pieces
        char promoted_pieces[] = {
            [Q] = 'q',
            [R] = 'r',
            [B] = 'b',
            [N] = 'n',
            [q] = 'q',
            [r] = 'r',
            [b] = 'b',
            [n] = 'n'
        };

    // piece bitboards
        U64 bitboards[12];

    // occupancy bitboards
        U64 occupancies[3];

    // side to move
        int side = -1;

    // enpassant square
        int enpassant = no_sq; 

    // castling rights
        int castle;

    // "almost" unique position identifier aka hash key or position key
        U64 hash_key;

    // repetition table 
        U64 repetition_table[1000];  // 1000 its the number of plies in a full game 

    // repetition index 
        int repetition_index;

    // half move counter
        int ply;

//////////////////////////////////////////////////////////// Time Controls Variables //////////////////////////////////////////////////////////////////////////////////////

    // exit from engine flag
        int quit = 0;

    // UCI "movestogo" command moves counter
        int movestogo = 30;

    // UCI "movetime" command time counter
        int movetime = -1;

    // UCI "time" command holder (ms)
        int time = -1;

    // UCI "inc" command's time increment holder
        int inc = 0;

    // UCI "starttime" command time holder
        int starttime = 0;

    // UCI "stoptime" command time holder
        int stoptime = 0;

    // variable to flag time control availability
        int timeset = 0;

    // variable to flag when the time is up
        int stopped = 0;

    // get time in milliseconds
        int get_time_ms(){ return GetTickCount(); }

  
    int input_waiting(){
    
        static int init = 0, pipe;
        static HANDLE inh;
        DWORD dw;

        if (!init){

            init = 1;
            inh = GetStdHandle(STD_INPUT_HANDLE);
            pipe = !GetConsoleMode(inh, &dw);

            if (!pipe){
                SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
                FlushConsoleInputBuffer(inh);
            }
        }
        
        if (pipe){
           if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
           return dw;
        }
        
        else{
           GetNumberOfConsoleInputEvents(inh, &dw);
           return dw <= 1 ? 0 : dw;
        }

   
    }

    // read GUI/user input
        void read_input(){

            // bytes to read holder
                int bytes;
    
            // GUI/user input
                char input[256] = "", *endc;

            // "listen" to STDIN
                if (input_waiting()){

                    // tell engine to stop calculating
                        stopped = 1;
        
                    // loop to read bytes from STDIN
                        do{              
                            // read bytes from STDIN
                                bytes=read(fileno(stdin), input, 256);
                            } while (bytes < 0); // until bytes available
        
        
                    // searches for the first occurrence of '\n'
                        endc = strchr(input,'\n');
        
                    // if found new line set value at pointer to 0
                        if (endc) *endc=0;
        
                    // if input is available
                        if (strlen(input) > 0){

                            // match UCI "quit" command
                                if (!strncmp(input, "quit", 4)){
                                    // tell engine to terminate execution    
                                        quit = 1;
                                }

                            // match UCI "stop" command
                                else if (!strncmp(input, "stop", 4)){
                                    // tell engine to terminate exacution
                                        quit = 1;
                                }
                        }   
                }
        }

    // a bridge function to interact between search and GUI input
        static void communicate(){

	        // if time is up break here
                if(timeset == 1 && get_time_ms() > stoptime) {
		            // tell engine to stop calculating
		                stopped = 1;
	            }
	
            // read GUI input
	            read_input();
        }

//////////////////////////////////////////////////////////// Random Numbers //////////////////////////////////////////////////////////////////////////////////////

    // pseudo random number state
        unsigned int random_state = 1804289383;

    // generate 32-bit pseudo legal numbers
        unsigned int get_random_U32_number(){

            // get current state
                unsigned int number = random_state;
    
            // XOR shift algorithm
                number ^= number << 13;
                number ^= number >> 17;
                number ^= number << 5;
    
            // update random number state
                random_state = number;
    
            // return random number
                return number;
        }


   // generate 64-bit pseudo legal numbers
        U64 get_random_U64_number(){
            // define 4 random numbers
                U64 n1, n2, n3, n4;
    
            // init random numbers slicing 16 bits from MS1B side
                n1 = (U64)(get_random_U32_number()) & 0xFFFF;
                n2 = (U64)(get_random_U32_number()) & 0xFFFF;
                n3 = (U64)(get_random_U32_number()) & 0xFFFF;
                n4 = (U64)(get_random_U32_number()) & 0xFFFF;
    
            // return random number
                return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
        }

    // generate magic number candidate
        U64 generate_magic_number(){
            return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
        }

//////////////////////////////////////////////////////////// Bit manipulations //////////////////////////////////////////////////////////////////////////////////////

    // count bits within a bitboard (Brian Kernighan's way)
        static inline int count_bits(U64 bitboard){

            // bit counter
                int count = 0;
    
            // consecutively reset least significant 1st bit
                while (bitboard){

                    // increment count
                        count++;
        
                    // reset least significant 1st bit
                        bitboard &= bitboard - 1;
                }
    
                // return bit count
                    return count;
        }

    // get least significant 1st bit index
        static inline int get_ls1b_index(U64 bitboard){

            // make sure bitboard is not 0
                if (bitboard){
                    // count trailing bits before LS1B
                        return count_bits((bitboard & -bitboard) - 1);
                }
    
            //otherwise
                else
                    // return illegal index
                        return -1;
        }

//////////////////////////////////////////////////////////// Zobrist Keys //////////////////////////////////////////////////////////////////////////////////////

    // random piece keys [piece][square]
        U64 piece_keys[12][64];

    // random enpassant keys [square]
        U64 enpassant_keys[64];

    // random castling keys
        U64 castle_keys[16];
    
    // random side key
        U64 side_key;

    // init random hash keys
        void init_random_keys(){

            // update pseudo random number state
                random_state = 1804289383;

            // loop over piece codes
                for(int piece = P; piece <= k; piece++ ){

                    // loop over board squares
                        for(int square = 0; square < 64; square++ )
                            // initialize random piece keys 
                                piece_keys[piece][square] = get_random_U64_number();
                                    
                }

            // loop over board squares
                for(int square = 0; square < 64; square++) 
                    // initialize random enpassant keys
                        enpassant_keys[square] = get_random_U64_number();

            // loop over castling keys
                for(int index = 0; index < 16; index++) 
                    // initialize random castling keys
                        castle_keys[index] = get_random_U64_number();
            
            // initialize random side key 
                side_key = get_random_U64_number();
        }


    // generate "almost" unique position ID aka hash key from scratch
        U64 generate_hash_key(){

            // final hash key
                U64 final_key = 0ULL;

            // temp piece bitboard copy 
                U64 bitboard;

            // loop over piece bitboards F
                for (int piece = P; piece <= k; piece++){

                    // initialize bitboard piece copy
                        bitboard = bitboards[piece];

                    // loop over the pieces within a bitboard 
                        while(bitboard){

                            // initialize square occupied by the piece 
                                int square = get_ls1b_index(bitboard);

                            // hash piece
                                final_key ^= piece_keys[piece][square];

                            // pop l1sb
                                pop_bit(bitboard, square);

                        }

                }

            // if enpassant square is on board 
                if(enpassant != no_sq)
                    // hash enpassant 
                        final_key ^= enpassant_keys[enpassant];

            // hash castling rights
                final_key ^= castle_keys[castle];

            // hash the side only if is black to move
                if(side == black) final_key ^= side_key;

            // return the hash key generated
                return final_key;
        }

//////////////////////////////////////////////////////////// Input/Output //////////////////////////////////////////////////////////////////////////////////////
    // print board
        void print_board(){

            // print offset
                printf("\n");

            // loop over board ranks
                for (int rank = 0; rank < 8; rank++){

                    // loop ober board files
                        for (int file = 0; file < 8; file++){

                            // init square
                                int square = rank * 8 + file;
            
                            // print ranks
                                if (!file)
                                    printf("  %d ", 8 - rank);
            
                            // define piece variable
                                int piece = -1;
            
                            // loop over all piece bitboards
                                for (int bb_piece = P; bb_piece <= k; bb_piece++){
                                    if (get_bit(bitboards[bb_piece], square))
                                        piece = bb_piece;
                                }

                            // print  piece 
                                printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
            
                        }
        
                    // print new line every rank
                        printf("\n");
                }
    
                    // print board files
                        printf("\n     a b c d e f g h\n\n");
    
                    // print side to move
                        printf("     Side:     %s\n", !side ? "white" : "black");
    
                    // print enpassant square
                        printf("     Enpassant:   %s\n", (enpassant != no_sq) ? square_to_coordinates[enpassant] : "no");
    
                    // print castling rights
                        printf("     Castling:  %c%c%c%c\n\n",  (castle & wk) ? 'K' : '-',
                                                                (castle & wq) ? 'Q' : '-',
                                                                (castle & bk) ? 'k' : '-',
                                                                (castle & bq) ? 'q' : '-');

            // print hash key
                printf("       hash key: %llx\n\n", hash_key);                                                
        }

        // parse FEN string
            void parse_fen(char *fen){

                // reset board position (bitboards)
                    memset(bitboards, 0ULL, sizeof(bitboards));
    
                // reset occupancies (bitboards)
                    memset(occupancies, 0ULL, sizeof(occupancies));
    
                // reset game state variables
                    side = 0;
                    enpassant = no_sq;
                    castle = 0;

                // reset repetition index
                    repetition_index = 0;

                // reset repetion table
                    memset(repetition_table, 0ULL, sizeof(repetition_table));

                // loop over board ranks
                    for (int rank = 0; rank < 8; rank++){

                        for (int file = 0; file < 8; file++){

                            // init current square
                                int square = rank * 8 + file;
            
                            // match ascii pieces within FEN string
                                if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')){

                                    // init piece type
                                        int piece = char_pieces[*fen];
                
                                    // set piece on corresponding bitboard
                                        set_bit(bitboards[piece], square);
                
                                    // increment pointer to FEN string
                                        fen++;
                                }
            
                            // match empty square numbers within FEN string
                                if (*fen >= '0' && *fen <= '9'){

                                    // init offset (convert char 0 to int 0)
                                        int offset = *fen - '0';
                
                                    // define piece variable
                                        int piece = -1;
                
                                    // loop over all piece bitboards
                                        for (int bb_piece = P; bb_piece <= k; bb_piece++){

                                            // if there is a piece on current square
                                                if (get_bit(bitboards[bb_piece], square))
                                            // get piece code
                                                piece = bb_piece;
                                        }
                
                                    // on empty current square
                                        if (piece == -1)
                                            // decrement file
                                                file--;
                
                                    // adjust file counter
                                        file += offset;
                
                                    // increment pointer to FEN string
                                        fen++;
                                }
            
                            // match rank separator
                                if (*fen == '/')
                                    // increment pointer to FEN string
                                        fen++;
                        }
                    }
    
        // got to parsing side to move (increment pointer to FEN string)
            fen++;
        
        // parse side to move
            (*fen == 'w') ? (side = white) : (side = black);
        
        // go to parsing castling rights
            fen += 2;
        
        // parse castling rights
            while (*fen != ' '){

            switch (*fen){
                case 'K': castle |= wk; break;
                case 'Q': castle |= wq; break;
                case 'k': castle |= bk; break;
                case 'q': castle |= bq; break;
                case '-': break;
            }

            // increment pointer to FEN string
            fen++;
        }
        
        // got to parsing enpassant square (increment pointer to FEN string)
            fen++;
        
        // parse enpassant square

            if (*fen != '-'){
                // parse enpassant file & rank
                    int file = fen[0] - 'a';
                    int rank = 8 - (fen[1] - '0');
            
                // init enpassant square
                    enpassant = rank * 8 + file;
            }
        
        // no enpassant square
            else
                enpassant = no_sq;
        
        // loop over white pieces bitboards
            for (int piece = P; piece <= K; piece++)
                // populate white occupancy bitboard
                    occupancies[white] |= bitboards[piece];
        
        // loop over black pieces bitboards
            for (int piece = p; piece <= k; piece++)
                // populate white occupancy bitboard
                    occupancies[black] |= bitboards[piece];
        
        // init all occupancies
            occupancies[both] |= occupancies[white];
            occupancies[both] |= occupancies[black];

        // initialize hash key
            hash_key = generate_hash_key();
    }


//////////////////////////////////////////////////////////// Attack Tables //////////////////////////////////////////////////////////////////////////////////////
    /* 
                Normal Representaion                           

        northwest    north   northeast              
        noWe         nort         noEa
                +7    +8    +9
                    \  |  /
        west    -1 <-  0 -> +1    east
                    /  |  \
                -9    -8    -7
        soWe         sout         soEa
        southwest    south   southeast


                Knight Representation   
            
                noNoWe    noNoEa
                    +15  +17
                     |     |
        noWeWe  +6 __|     |__+10  noEaEa
                      \   /
                       >0<
                   __ /   \ __
        soWeWe -10   |     |   -6  soEaEa
                     |     |
                    -17  -15
                soSoWe    soSoEa

    */
   
    // const used to make sure pawns cannot capture between A & F and H & F 
        const U64 notAFile = 0xfefefefefefefefe;
        const U64 notHFile = 0x7f7f7f7f7f7f7f7f; 
        const U64 notGHFile = 0x3F3F3F3F3F3F3F3F;
        const U64 notABFile = 0xFCFCFCFCFCFCFCFC;

   // normal representation
        U64 soutOne (U64 bitboard) {return  bitboard >> 8;}
        U64 nortOne (U64 bitboard) {return  bitboard << 8;}

        U64 eastOne (U64 bitboard) {return (bitboard & notHFile) << 1;}
        U64 noEaOne (U64 bitboard) {return (bitboard & notHFile) << 9;}
        U64 soEaOne (U64 bitboard) {return (bitboard & notHFile) >> 7;}

        U64 westOne (U64 bitboard) {return (bitboard & notAFile) >> 1;}
        U64 soWeOne (U64 bitboard) {return (bitboard & notAFile) >> 9;}
        U64 noWeOne (U64 bitboard) {return (bitboard & notAFile) << 7;}

    // knight representation
        U64 noNoEa(U64 bitboard) {return (bitboard << 17) & notAFile ;}
        U64 noEaEa(U64 bitboard) {return (bitboard << 10) & notABFile;}
        U64 soEaEa(U64 bitboard) {return (bitboard >>  6) & notABFile;}
        U64 soSoEa(U64 bitboard) {return (bitboard >> 15) & notAFile ;}
        U64 noNoWe(U64 bitboard) {return (bitboard << 15) & notHFile ;}
        U64 noWeWe(U64 bitboard) {return (bitboard <<  6) & notGHFile;}
        U64 soWeWe(U64 bitboard) {return (bitboard >> 10) & notGHFile;}
        U64 soSoWe(U64 bitboard) {return (bitboard >> 17) & notHFile ;}

        U64 rnoNoEa(U64 bitboard) {return (bitboard << 17) & notHFile ;}
        U64 rnoEaEa(U64 bitboard) {return (bitboard << 10) & notGHFile;}
        U64 rsoEaEa(U64 bitboard) {return (bitboard >> 6 ) & notGHFile;}
        U64 rsoSoEa(U64 bitboard) {return (bitboard >> 15) & notHFile ;}
        U64 rnoNoWe(U64 bitboard) {return (bitboard << 15) & notAFile ;}
        U64 rnoWeWe(U64 bitboard) {return (bitboard <<  6) & notABFile;}
        U64 rsoWeWe(U64 bitboard) {return (bitboard >> 10) & notABFile;}
        U64 rsoSoWe(U64 bitboard) {return (bitboard >> 17 ) & notAFile;}

    // bishop relevant occupancy bit count for every square on board
        const int bishop_relevant_bits[64] = {
            6, 5, 5, 5, 5, 5, 5, 6, 
            5, 5, 5, 5, 5, 5, 5, 5, 
            5, 5, 7, 7, 7, 7, 5, 5, 
            5, 5, 7, 9, 9, 7, 5, 5, 
            5, 5, 7, 9, 9, 7, 5, 5, 
            5, 5, 7, 7, 7, 7, 5, 5, 
            5, 5, 5, 5, 5, 5, 5, 5, 
            6, 5, 5, 5, 5, 5, 5, 6
        };

    // rook relevant occupancy bit count for every square on board
        const int rook_relevant_bits[64] = {
            12, 11, 11, 11, 11, 11, 11, 12, 
            11, 10, 10, 10, 10, 10, 10, 11, 
            11, 10, 10, 10, 10, 10, 10, 11, 
            11, 10, 10, 10, 10, 10, 10, 11, 
            11, 10, 10, 10, 10, 10, 10, 11, 
            11, 10, 10, 10, 10, 10, 10, 11, 
            11, 10, 10, 10, 10, 10, 10, 11, 
            12, 11, 11, 11, 11, 11, 11, 12
        };

    // rook magic numbers
        U64 rook_magic_numbers[64] = {
            0x8a80104000800020ULL,
            0x140002000100040ULL,
            0x2801880a0017001ULL,
            0x100081001000420ULL,
            0x200020010080420ULL,
            0x3001c0002010008ULL,
            0x8480008002000100ULL,
            0x2080088004402900ULL,
            0x800098204000ULL,
            0x2024401000200040ULL,
            0x100802000801000ULL,
            0x120800800801000ULL,
            0x208808088000400ULL,
            0x2802200800400ULL,
            0x2200800100020080ULL,
            0x801000060821100ULL,
            0x80044006422000ULL,
            0x100808020004000ULL,
            0x12108a0010204200ULL,
            0x140848010000802ULL,
            0x481828014002800ULL,
            0x8094004002004100ULL,
            0x4010040010010802ULL,
            0x20008806104ULL,
            0x100400080208000ULL,
            0x2040002120081000ULL,
            0x21200680100081ULL,
            0x20100080080080ULL,
            0x2000a00200410ULL,
            0x20080800400ULL,
            0x80088400100102ULL,
            0x80004600042881ULL,
            0x4040008040800020ULL,
            0x440003000200801ULL,
            0x4200011004500ULL,
            0x188020010100100ULL,
            0x14800401802800ULL,
            0x2080040080800200ULL,
            0x124080204001001ULL,
            0x200046502000484ULL,
            0x480400080088020ULL,
            0x1000422010034000ULL,
            0x30200100110040ULL,
            0x100021010009ULL,
            0x2002080100110004ULL,
            0x202008004008002ULL,
            0x20020004010100ULL,
            0x2048440040820001ULL,
            0x101002200408200ULL,
            0x40802000401080ULL,
            0x4008142004410100ULL,
            0x2060820c0120200ULL,
            0x1001004080100ULL,
            0x20c020080040080ULL,
            0x2935610830022400ULL,
            0x44440041009200ULL,
            0x280001040802101ULL,
            0x2100190040002085ULL,
            0x80c0084100102001ULL,
            0x4024081001000421ULL,
            0x20030a0244872ULL,
            0x12001008414402ULL,
            0x2006104900a0804ULL,
            0x1004081002402ULL
        };

    // bishop magic numbers
        U64 bishop_magic_numbers[64] = {
            0x40040844404084ULL,
            0x2004208a004208ULL,
            0x10190041080202ULL,
            0x108060845042010ULL,
            0x581104180800210ULL,
            0x2112080446200010ULL,
            0x1080820820060210ULL,
            0x3c0808410220200ULL,
            0x4050404440404ULL,
            0x21001420088ULL,
            0x24d0080801082102ULL,
            0x1020a0a020400ULL,
            0x40308200402ULL,
            0x4011002100800ULL,
            0x401484104104005ULL,
            0x801010402020200ULL,
            0x400210c3880100ULL,
            0x404022024108200ULL,
            0x810018200204102ULL,
            0x4002801a02003ULL,
            0x85040820080400ULL,
            0x810102c808880400ULL,
            0xe900410884800ULL,
            0x8002020480840102ULL,
            0x220200865090201ULL,
            0x2010100a02021202ULL,
            0x152048408022401ULL,
            0x20080002081110ULL,
            0x4001001021004000ULL,
            0x800040400a011002ULL,
            0xe4004081011002ULL,
            0x1c004001012080ULL,
            0x8004200962a00220ULL,
            0x8422100208500202ULL,
            0x2000402200300c08ULL,
            0x8646020080080080ULL,
            0x80020a0200100808ULL,
            0x2010004880111000ULL,
            0x623000a080011400ULL,
            0x42008c0340209202ULL,
            0x209188240001000ULL,
            0x400408a884001800ULL,
            0x110400a6080400ULL,
            0x1840060a44020800ULL,
            0x90080104000041ULL,
            0x201011000808101ULL,
            0x1a2208080504f080ULL,
            0x8012020600211212ULL,
            0x500861011240000ULL,
            0x180806108200800ULL,
            0x4000020e01040044ULL,
            0x300000261044000aULL,
            0x802241102020002ULL,
            0x20906061210001ULL,
            0x5a84841004010310ULL,
            0x4010801011c04ULL,
            0xa010109502200ULL,
            0x4a02012000ULL,
            0x500201010098b028ULL,
            0x8040002811040900ULL,
            0x28000010020204ULL,
            0x6000020202d0240ULL,
            0x8918844842082200ULL,
            0x4010011029020020ULL
        };
        
    // pawn attacks[side][square]
        U64 pawn_attacks[2][64];

    // knight attacks table [square]
        U64 knight_attacks[64];

    // king attacks table [square]
        U64 king_attacks[64];
        
    // bishop attack masks
        U64 bishop_masks[64];
    
    // rook attack masks
        U64 rook_masks[64];

    // bishop attacks table [square][occupancies]
        U64 bishop_attacks[64][512];
    
    // rook attacks rable [square][occupancies]
        U64 rook_attacks[64][4096];


    // generate pawn attacks
        U64 mask_pawn_attacks(int side, int square){

            // result attacks bitboard
                U64 attacks = 0ULL;

            // piece bitboard
                U64 bitboard = 0ULL;
    
            // set piece on board
                set_bit(bitboard, square);
    
            // white pawns
                if (!side){

                    // generate pawn attacks
                        if (soEaOne (bitboard)) attacks |= soEaOne (bitboard);
                        if (soWeOne (bitboard)) attacks |= soWeOne (bitboard);
                }
    
            // black pawns
                else{
                    // generate pawn attacks
                        if (noWeOne (bitboard)) attacks |= noWeOne (bitboard);
                        if (noEaOne (bitboard)) attacks |= noEaOne (bitboard);      
                    }
    
            // return attack map
                return attacks;
        }


            
    // generate knight attacks
        U64 mask_knight_attacks(int square){

            // result attacks bitboard
                U64 attacks = 0ULL;

            // piece bitboard
                U64 bitboard = 0ULL;
    
            // set piece on board
                set_bit(bitboard, square);
    
            // generate knight attacks
                if (soSoWe(bitboard)) attacks |= (soSoWe(bitboard));
                if (soSoEa(bitboard)) attacks |= (soSoEa(bitboard));
                if (soWeWe(bitboard)) attacks |= (soWeWe(bitboard));
                if (soEaEa(bitboard)) attacks |= (soEaEa(bitboard));

                if (noNoWe(bitboard)) attacks |= (noNoWe(bitboard));
                if (noNoEa(bitboard)) attacks |= (noNoEa(bitboard));
                if (noWeWe(bitboard)) attacks |= (noWeWe(bitboard));
                if (noEaEa(bitboard)) attacks |= (noEaEa(bitboard));

            // return attack map
                return attacks;
        }

    // generate knight attacks    
        U64 mask_king_attacks(int square){

            // result attacks bitboard
                U64 attacks = 0ULL;

            // piece bitboard
                U64 bitboard = 0ULL;
    
            // set piece on board
                set_bit(bitboard, square);
    
            // generate king attacks
                if (soutOne (bitboard)) attacks |= (soutOne (bitboard));
                if (soWeOne (bitboard)) attacks |= (soWeOne (bitboard));
                if (soEaOne (bitboard)) attacks |= (soEaOne (bitboard));
                if (westOne (bitboard)) attacks |= (westOne (bitboard));
                                
                if (nortOne (bitboard)) attacks |= (nortOne (bitboard));
                if (noEaOne (bitboard)) attacks |= (noEaOne (bitboard));
                if (noWeOne (bitboard)) attacks |= (noWeOne (bitboard));
                if (eastOne (bitboard)) attacks |= (eastOne (bitboard));    
    
            // return attack map
                return attacks;
        }
    
    // mask bishop attacks
        U64 mask_bishop_attacks(int square){

            // result attacks bitboard
                U64 attacks = 0ULL;
    
            // init ranks & files
                int r, f;
    
            // init target rank & files
                int tr = square / 8;
                int tf = square % 8;
    
            // mask relevant bishop occupancy bits
                for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
                for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
                for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
                for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));
    
            // return attack map
                return attacks;
        }

    // mask rook attacks
        U64 mask_rook_attacks(int square){

            // result attacks bitboard
                U64 attacks = 0ULL;
    
            // init ranks & files
                int r, f;
    
            // init target rank & files
                int tr = square / 8;
                int tf = square % 8;
    
            // mask relevant rook occupancy bits
                for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
                for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
                for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
                for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));
    
            // return attack map
                return attacks;
        }

    // generate bishop attacks on the fly
        U64 bishop_attacks_on_the_fly(int square, U64 block){

            // result attacks bitboard
                U64 attacks = 0ULL;
    
            // init ranks & files
                int r, f;
    
            // init target rank & files
                int tr = square / 8;
                int tf = square % 8;
    
            // generate bishop atacks
                for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++){
                    attacks |= (1ULL << (r * 8 + f));
                    if ((1ULL << (r * 8 + f)) & block) break;
                }
    
                for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++){
                    attacks |= (1ULL << (r * 8 + f));
                    if ((1ULL << (r * 8 + f)) & block) break;
                }
    
                for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--){
                    attacks |= (1ULL << (r * 8 + f));
                    if ((1ULL << (r * 8 + f)) & block) break;
                }
    
                for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--){
                    attacks |= (1ULL << (r * 8 + f));
                    if ((1ULL << (r * 8 + f)) & block) break;
                }
    
            // return attack map
                return attacks;
        }

    // generate rook attacks on the fly
        U64 rook_attacks_on_the_fly(int square, U64 block){

            // result attacks bitboard
                U64 attacks = 0ULL;
    
            // init ranks & files
                int r, f;
    
            // init target rank & files
                int tr = square / 8;
                int tf = square % 8;
    
            // generate rook attacks
                for (r = tr + 1; r <= 7; r++){
                    attacks |= (1ULL << (r * 8 + tf));
                    if ((1ULL << (r * 8 + tf)) & block) break;
                }
    
                for (r = tr - 1; r >= 0; r--){
                    attacks |= (1ULL << (r * 8 + tf));
                    if ((1ULL << (r * 8 + tf)) & block) break;
                }
    
                for (f = tf + 1; f <= 7; f++){
                    attacks |= (1ULL << (tr * 8 + f));
                    if ((1ULL << (tr * 8 + f)) & block) break;
                }
                
                for (f = tf - 1; f >= 0; f--){
                    attacks |= (1ULL << (tr * 8 + f));
                    if ((1ULL << (tr * 8 + f)) & block) break;
                }
                
            // return attack map
                return attacks;
        }

    // init leaper pieces attacks
        void init_leapers_attacks(){

            // loop over 64 board squares
                for (int square = 0; square < 64; square++){
                    // init pawn attacks
                        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
                        pawn_attacks[black][square] = mask_pawn_attacks(black, square);
            
                    // init knight attacks
                        knight_attacks[square] = mask_knight_attacks(square);
            
                    // init king attacks
                        king_attacks[square] = mask_king_attacks(square);
                }
        }

    // set occupancies
        U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask){

            // occupancy map
                U64 occupancy = 0ULL;
    
            // loop over the range of bits within attack mask
                for (int count = 0; count < bits_in_mask; count++){

                    // get LS1B index of attacks mask
                        int square = get_ls1b_index(attack_mask);
        
                    // pop LS1B in attack map
                        pop_bit(attack_mask, square);
        
                    // make sure occupancy is on board
                        if (index & (1 << count))
                    // populate occupancy map
                        occupancy |= (1ULL << square);
                }
    
            // return occupancy map
                return occupancy;
        }

//////////////////////////////////////////////////////////// Magic //////////////////////////////////////////////////////////////////////////////////////


    // find appropriate magic number
        U64 find_magic_number(int square, int relevant_bits, int bishop){

            // init occupancies
                U64 occupancies[4096];
    
            // init attack tables
                U64 attacks[4096];
    
            // init used attacks
                U64 used_attacks[4096];
    
            // init attack mask for a current piece
                U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);
    
            // init occupancy indicies
                int occupancy_indicies = 1 << relevant_bits;
    
            // loop over occupancy indicies
                for (int index = 0; index < occupancy_indicies; index++){

                    // init occupancies
                        occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);
        
                    // init attacks
                        attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies[index]) :
                                                  rook_attacks_on_the_fly(square, occupancies[index]);
                }
    
            // test magic numbers loop
                for (int random_count = 0; random_count < 100000000; random_count++){

                    // generate magic number candidate
                        U64 magic_number = generate_magic_number();
        
                    // skip inappropriate magic numbers
                        if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;
        
                    // init used attacks
                        memset(used_attacks, 0ULL, sizeof(used_attacks));
        
                    // init index & fail flag
                        int index, fail;
        
                    // test magic index loop
                        for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++){

                            // init magic index
                                int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));
            
                            // if magic index works
                                if (used_attacks[magic_index] == 0ULL)
                                    // init used attacks
                                        used_attacks[magic_index] = attacks[index];
            
                            // otherwise
                                else if (used_attacks[magic_index] != attacks[index])
                                    // magic index doesn't work
                                        fail = 1;
                        }
        
                    // if magic number works
                        if (!fail)

                    // return it
                        return magic_number;
                }
    
            // if magic number doesn't work
                printf("  Magic number fails!\n");
                return 0ULL;
        }

    // init magic numbers
        void init_magic_numbers(){

            // loop over 64 board squares
                for (int square = 0; square < 64; square++)
                    // init rook magic numbers
                        rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);

            // loop over 64 board squares
                for (int square = 0; square < 64; square++)
                    // init bishop magic numbers
                        bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
        }

    // init slider piece's attack tables
        void init_sliders_attacks(int bishop){

            // loop over 64 board squares
                for (int square = 0; square < 64; square++){

                    // init bishop & rook masks
                        bishop_masks[square] = mask_bishop_attacks(square);
                        rook_masks[square] = mask_rook_attacks(square);
                
                    // init current mask
                        U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];
                
                    // init relevant occupancy bit count
                        int relevant_bits_count = count_bits(attack_mask);
                
                    // init occupancy indicies
                        int occupancy_indicies = (1 << relevant_bits_count);
                
                    // loop over occupancy indicies
                        for (int index = 0; index < occupancy_indicies; index++){

                            // bishop
                                if (bishop){

                                    // init current occupancy variation
                                        U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                        
                                    // init magic index
                                        int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
                        
                                    // init bishop attacks
                                        bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
                                }
                    
                            // rook
                                else{

                                    // init current occupancy variation
                                        U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                        
                                    // init magic index
                                        int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
                        
                                    // init bishop attacks
                                        rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
                    
                                    }
                        }
                }
        }

    // get bishop attacks
        static inline U64 get_bishop_attacks(int square, U64 occupancy){

            // get bishop attacks assuming current board occupancy
                occupancy &= bishop_masks[square];
                occupancy *= bishop_magic_numbers[square];
                occupancy >>= 64 - bishop_relevant_bits[square];
            
            // return bishop attacks
                return bishop_attacks[square][occupancy];
        }

    // get rook attacks
        static inline U64 get_rook_attacks(int square, U64 occupancy){

            // get bishop attacks assuming current board occupancy
                occupancy &= rook_masks[square];
                occupancy *= rook_magic_numbers[square];
                occupancy >>= 64 - rook_relevant_bits[square];
            
            // return rook attacks
                return rook_attacks[square][occupancy];
        }

    static inline U64 get_queen_attacks(int square, U64 occupancy){

        // init result attacks bitboard
            U64 queen_attacks = 0ULL;
    
        // init bishop occupancies
            U64 bishop_occupancy = occupancy;
    
        // init rook occupancies
            U64 rook_occupancy = occupancy;
    
        // get bishop attacks assuming current board occupancy
            bishop_occupancy &= bishop_masks[square];
            bishop_occupancy *= bishop_magic_numbers[square];
            bishop_occupancy >>= 64 - bishop_relevant_bits[square];
    
        // get bishop attacks
            queen_attacks = bishop_attacks[square][bishop_occupancy];
    
        // get bishop attacks assuming current board occupancy
            rook_occupancy &= rook_masks[square];
            rook_occupancy *= rook_magic_numbers[square];
            rook_occupancy >>= 64 - rook_relevant_bits[square];
    
        // get rook attacks
            queen_attacks |= rook_attacks[square][rook_occupancy];
    
        // return queen attacks
            return queen_attacks;
    }

//////////////////////////////////////////////////////////// Move Generator //////////////////////////////////////////////////////////////////////////////////////


    // enconde move
        #define encode_move(source, target, piece, promoted, capture, double, enpassant, castling) \
            (source) |          \
            (target << 6) |     \
            (piece << 12) |     \
            (promoted << 16) |  \
            (capture << 20) |   \
            (double << 21) |    \
            (enpassant << 22) | \
            (castling << 23)    \

    // extract source square
        #define get_move_source(move) (move & 0x3f)

    // extract target square
        #define get_move_target(move) ((move & 0xfc0) >> 6)

    // extract piece
        #define get_move_piece(move) ((move & 0xf000) >> 12)

    // extract promoted piece
        #define get_move_promoted(move) ((move & 0xf0000) >> 16)

    // extract capture flag 
        #define get_move_capture(move) (move & 0x100000)

   // extract double pawn push flag
        #define get_move_double(move) (move & 0x200000)

    // extract enpassant flag
        #define get_move_enpassant(move) (move & 0x400000)

    // extract castling flag
        #define get_move_castling(move) (move & 0x800000)

    // move list structure 
        typedef struct{

            // moves
                int moves[256];

            // move count 
                int count; 

        } moves;

    // is square current given attacked by the current given side
        static inline int is_square_attacked(int square, int side){

            // attacked by white pawns
                if ((side == white) && (pawn_attacks[black][square] & bitboards[P])) return 1;
            
            // attacked by black pawns
                if ((side == black) && (pawn_attacks[white][square] & bitboards[p])) return 1;
            
            // attacked by knights
                if (knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;
            
            // attacked by bishops
                if (get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return 1;

            // attacked by rooks
                if (get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return 1;    

            // attacked by bishops
                if (get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q])) return 1;
            
            // attacked by kings
                if (king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;

            // by default return false
                return 0;
        }

    // print attacked squares
        void print_attacked_squares(int side){

            printf("\n");
        
            // loop over board ranks
                for (int rank = 0; rank < 8; rank++){
                    // loop over board files
                        for (int file = 0; file < 8; file++){

                            // init square
                                int square = rank * 8 + file;
                    
                            // print ranks
                                if (!file)
                                    printf("  %d ", 8 - rank);
                    
                            // check whether current square is attacked or not
                                printf(" %d", is_square_attacked(square, side) ? 1 : 0);
                        }
                
                    // print new line every rank
                        printf("\n");
                }
            
            // print files
                printf("\n     a b c d e f g h\n\n");
        }

    // add move to the move list
        static inline void add_move(moves *move_list, int move){

            // store move
                move_list->moves[move_list->count] = move;
    
            // increment move count
                move_list->count++;
        }

        // print move (for UCI purposes)
            void print_move(int move){

                printf("%s%s%c\n",  square_to_coordinates[get_move_source(move)],
                                    square_to_coordinates[get_move_target(move)],
                                    promoted_pieces[get_move_promoted(move)]);
            }


    // print move list
        void print_move_list(moves *move_list){

            // do nothing on empty move list
                if (!move_list->count){
                    printf("\n     No move in the move list!\n");
                    return;
                }
    
            printf("\n     move    piece     capture   double    enpass    castling\n\n");
    
            // loop over moves within a move list
                for (int move_count = 0; move_count < move_list->count; move_count++){
                    
                    // init move
                        int move = move_list->moves[move_count];
        
                    // print move
                        printf("     %s%s%c     %c         %d         %d         %d         %d\n", square_to_coordinates[get_move_source(move)],
                                                                                                   square_to_coordinates[get_move_target(move)],
                                                                                                   get_move_promoted(move) ? promoted_pieces[get_move_promoted(move)] : ' ',
                                                                                                   ascii_pieces[get_move_piece(move)],
                                                                                                   get_move_capture(move) ? 1 : 0,
                                                                                                   get_move_double(move) ? 1 : 0,
                                                                                                   get_move_enpassant(move) ? 1 : 0,
                                                                                                   get_move_castling(move) ? 1 : 0);
    
                }
    
            // print total number of moves
                printf("\n\n     Total number of moves: %d\n\n", move_list->count);
        }

        // preserve board state
            #define copy_board()                                                      \
                U64 bitboards_copy[12], occupancies_copy[3];                          \
                int side_copy, enpassant_copy, castle_copy;                           \
                memcpy(bitboards_copy, bitboards, 96);                                \
                memcpy(occupancies_copy, occupancies, 24);                            \
                side_copy = side, enpassant_copy = enpassant, castle_copy = castle;   \
                U64 hash_key_copy = hash_key;                                         \

        // restore board state
            #define take_back()                                                       \
                memcpy(bitboards, bitboards_copy, 96);                                \
                memcpy(occupancies, occupancies_copy, 24);                            \
                side = side_copy, enpassant = enpassant_copy, castle = castle_copy;   \
                hash_key = hash_key_copy                                              \

    // move types
        enum { all_moves, only_captures };

    /*
                            castling   move     in      in
                                right update     binary  decimal

    king & rooks didn't move:    1111 & 1111  =  1111    15

            white king  moved:   1111 & 1100  =  1100    12
    white king's rook moved:     1111 & 1110  =  1110    14
    white queen's rook moved:    1111 & 1101  =  1101    13
        
            black king moved:    1111 & 0011  =  1011    3
    black king's rook moved:     1111 & 1011  =  1011    11
    black queen's rook moved:    1111 & 0111  =  0111    7

    */

    // castling rights update constants
        const int castling_rights[64] = {
            7, 15, 15, 15,  3, 15, 15, 11,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            13, 15, 15, 15, 12, 15, 15, 14
        };

    // make move on chess board
        static inline int make_move(int move, int move_flag){

            // quite moves
                if (move_flag == all_moves){

                    // preserve board state
                        copy_board();
        
                    // parse move
                        int source_square = get_move_source(move);
                        int target_square = get_move_target(move);
                        int piece = get_move_piece(move);
                        int promoted_piece = get_move_promoted(move);
                        int capture = get_move_capture(move);
                        int double_push = get_move_double(move);
                        int enpass = get_move_enpassant(move);
                        int castling = get_move_castling(move);
                
                    // move piece
                        pop_bit(bitboards[piece], source_square);
                        set_bit(bitboards[piece], target_square);

                    // hash piece
                        hash_key ^= piece_keys[piece][source_square]; // remove piece from source square in hash key
                        hash_key ^= piece_keys[piece][target_square]; // set the piece to the target square in hash key

                    // handling capture moves
                        if (capture){

                            // pick up bitboard piece index ranges depending on side
                                int start_piece, end_piece;
                    
                            // white to move
                                if (side == white){
                                    start_piece = p;
                                    end_piece = k;
                                }
                    
                            // black to move
                                else{
                                    start_piece = P;
                                    end_piece = K;
                                }
                    
                            // loop over bitboards opposite to the current side to move
                                for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++){

                                    // if there's a piece on the target square
                                        if (get_bit(bitboards[bb_piece], target_square)){

                                            // remove it from corresponding bitboard
                                                pop_bit(bitboards[bb_piece], target_square);
                                            
                                            // remove the piece from hash key 
                                                hash_key ^= piece_keys[bb_piece][target_square];
                                                break;
                                        }
                                }
                        }
                
                    // handle pawn promotions
                        if (promoted_piece){

                            // erase the pawn from the target square
                                //pop_bit(bitboards[(side == white) ? P : p], target_square);
                    
                            // white to move
                                if(side == white){

                                    // erase the pawn from the target square
                                        pop_bit(bitboards[P], target_square);

                                    // remove pawn from hash key 
                                        hash_key ^= piece_keys[P][target_square];

                                }

        
                            // black to move
                                else{

                                    // erase the pawn from the target square
                                        pop_bit(bitboards[p], target_square);

                                    // remove pawn from hash key 
                                        hash_key ^= piece_keys[p][target_square];
                                }

                            // set up promoted piece on chess board
                                set_bit(bitboards[promoted_piece], target_square);

                            // add promoted piece into the hash key 
                                hash_key ^= piece_keys[promoted_piece][target_square];
                        }
                
                    // handle enpassant captures
                        if (enpass){

                        // erase the pawn depending on side to move
                            (side == white) ? pop_bit(bitboards[p], target_square + 8) :
                                              pop_bit(bitboards[P], target_square - 8);

                        // white to move
                            if(side == white){

                                // remove capture pawn 
                                    pop_bit(bitboards[p], target_square + 8);

                                // remove pawn from hash key
                                    hash_key ^= piece_keys[p][target_square + 8];

                            }

                            else{

                                // remove capture pawn 
                                    pop_bit(bitboards[P], target_square - 8);

                                // remove pawn from hash key
                                    hash_key ^= piece_keys[P][target_square - 8];
                            }
                        }

                    // hash enpassant if available (remove enpassant square from hash key)
                        if(enpassant != no_sq) hash_key ^= enpassant_keys[enpassant];
                
                    // reset enpassant square
                        enpassant = no_sq;

                    // handle double pawn push
                        if (double_push){

                        // set enpassant aquare depending on side to move
                            //(side == white) ? (enpassant = target_square + 8) :
                                              //(enpassant = target_square - 8);

                        // white to move
                            if(side == white){

                                // set enpassant square
                                    enpassant = target_square + 8;

                                // hash enpassant 
                                    hash_key ^= enpassant_keys[target_square + 8];
                            }
                        // black to move
                            else{

                                // set enpassant square
                                    enpassant = target_square - 8;

                                // hash enpassant 
                                    hash_key ^= enpassant_keys[target_square - 8];

                            }
                        }
                
                    // handle castling moves
                        if (castling){

                            // switch target square
                                switch (target_square){

                                    // white castles king side
                                        case (g1):
                                            // move H rook
                                                pop_bit(bitboards[R], h1);
                                                set_bit(bitboards[R], f1);
                                            
                                            // hash rook 
                                                hash_key ^= piece_keys[R][h1]; // remove rook from h1 from hash key
                                                hash_key ^= piece_keys[R][f1]; // put rook on f1 into a hash key    
                                        break;
                        
                                    // white castles queen side
                                        case (c1):
                                            // move A rook
                                                pop_bit(bitboards[R], a1);
                                                set_bit(bitboards[R], d1);

                                            // hash rook 
                                                hash_key ^= piece_keys[R][a1]; // remove rook from h1 from hash key
                                                hash_key ^= piece_keys[R][d1]; // put rook on f1 into a hash key
                                        break;
                        
                                    // black castles king side
                                        case (g8):
                                            // move H rook
                                                pop_bit(bitboards[r], h8);
                                                set_bit(bitboards[r], f8);

                                            // hash rook 
                                                hash_key ^= piece_keys[r][h8]; // remove rook from h8 from hash key
                                                hash_key ^= piece_keys[r][f8]; // put rook on f8 into a hash key    
                                        break;
                        
                                    // black castles queen side
                                        case (c8):
                                            // move A rook
                                                pop_bit(bitboards[r], a8);
                                                set_bit(bitboards[r], d8);

                                             // hash rook 
                                                hash_key ^= piece_keys[r][a8]; // remove rook from a8 from hash key
                                                hash_key ^= piece_keys[r][d8]; // put rook on d8 into a hash key   
                                        break;
                                }
                        }
                    
                    // hash castling
                        hash_key ^= castle_keys[castle];
                
                    // update castling rights
                        castle &= castling_rights[source_square];
                        castle &= castling_rights[target_square];

                    // hash castling
                        hash_key ^= castle_keys[castle];
                
                    // reset occupancies
                        memset(occupancies, 0ULL, 24);
                
                    // loop over white pieces bitboards
                        for (int bb_piece = P; bb_piece <= K; bb_piece++)
                            // update white occupancies
                                occupancies[white] |= bitboards[bb_piece];

                    // loop over black pieces bitboards
                        for (int bb_piece = p; bb_piece <= k; bb_piece++)
                            // update black occupancies
                                occupancies[black] |= bitboards[bb_piece];

                            // update both sides occupancies
                                occupancies[both] |= occupancies[white];
                                occupancies[both] |= occupancies[black];
                
                            // change side
                                side ^= 1;

                            // hash side
                                hash_key ^= side_key;

                            // debug hask key incremental updates 
                            // build hash key for the updated position(after the move is made) from scratch
                                /*U64 hash_from_scratch = generate_hash_key();

                            /// in case if hash key built from scratch dosen´t match 
                            /// the on that was incrementally update we interrupt execution 
                                if(hash_key != hash_from_scratch){
                                    
                                    printf("\n\nMake Move\n");
                                    printf("move: "); print_move(move);
                                    print_board();
                                    printf("hash key should be: %llx\n", hash_from_scratch);
                                    getchar();

                                }*/
                
                            // make sure that king has not been exposed into a check
                                if (is_square_attacked((side == white) ? get_ls1b_index(bitboards[k]) : get_ls1b_index(bitboards[K]), side)){

                                    // take move back
                                        take_back();
                    
                                    // return illegal move
                                        return 0;
                                }
                
                    //
                        else
                            // return legal move
                                return 1;
                    
                    
                }
    
        // capture moves
            else{

                // make sure move is the capture
                    if (get_move_capture(move))
                        make_move(move, all_moves);
        
                // otherwise the move is not a capture
                    else
                        // don't make it
                            return 0;
            }
    }

    // generate all moves
        static inline void generate_moves(moves *move_list){

            // init move count
                move_list->count = 0;

            // define source & target squares
                int source_square, target_square;
            
            // define current piece's bitboard copy & it's attacks
                U64 bitboard, attacks;
            
            // loop over all the bitboards
                for (int piece = P; piece <= k; piece++){

                    // init piece bitboard copy
                        bitboard = bitboards[piece];
                
                    // generate white pawns & white king castling moves
                        if (side == white){

                        // pick up white pawn bitboards index
                            if (piece == P){

                                // loop over white pawns within white pawn bitboard
                                    while (bitboard){

                                        // init source square
                                            source_square = get_ls1b_index(bitboard);
                                
                                        // init target square
                                            target_square = source_square - 8;
                                
                                        // generate quite pawn moves
                                            if (!(target_square < a8) && !get_bit(occupancies[both], target_square)){

                                            // pawn promotion
                                                if (source_square >= a7 && source_square <= h7){                            
                                                    add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
                                                    add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
                                                    add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
                                                    add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
                                                }
                                    
                                                else{

                                                    // one square ahead pawn move
                                                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                        
                                                    // two squares ahead pawn move
                                                        if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square - 8))
                                                            add_move(move_list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));
                                                }
                                            }
                            
                                        // init pawn attacks bitboard
                                            attacks = pawn_attacks[side][source_square] & occupancies[black];
                            
                                        // generate pawn captures
                                            while (attacks){

                                                // init target square
                                                    target_square = get_ls1b_index(attacks);
                                
                                                // pawn promotion
                                                    if (source_square >= a7 && source_square <= h7){
                                                        add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
                                                        add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
                                                        add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
                                                        add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
                                                    }
                                
                                                    else
                                                        // one square ahead pawn move
                                                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                
                                                // pop ls1b of the pawn attacks
                                                    pop_bit(attacks, target_square);
                                            }
                            
                                        // generate enpassant captures
                                            if (enpassant != no_sq){

                                                // lookup pawn attacks and bitwise AND with enpassant square (bit)
                                                    U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                                
                                                // make sure enpassant capture available
                                                    if (enpassant_attacks){
                                                        // init enpassant capture target square
                                                            int target_enpassant = get_ls1b_index(enpassant_attacks);
                                                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                                                    }
                                            }
                            
                                        // pop ls1b from piece bitboard copy
                                            pop_bit(bitboard, source_square);
                                    }
                            }
                    
                            // castling moves
                                if (piece == K){

                                    // king side castling is available
                                        if (castle & wk){

                                            // make sure square between king and king's rook are empty
                                                if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1)){
                                                    // make sure king and the f1 squares are not under attacks
                                                        if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
                                                            add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
                                                }
                                        }
                        
                                    // queen side castling is available
                                        if (castle & wq){
                                            // make sure square between king and queen's rook are empty
                                                if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1)){

                                                    // make sure king and the d1 squares are not under attacks
                                                        if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
                                                            add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
                                                }
                                        }
                                }
                        }
                
                    // generate black pawns & black king castling moves
                        else{

                            // pick up black pawn bitboards index
                                if (piece == p){

                                    // loop over white pawns within white pawn bitboard
                                        while (bitboard){

                                            // init source square
                                                source_square = get_ls1b_index(bitboard);
                            
                                            // init target square
                                                target_square = source_square + 8;
                            
                                            // generate quite pawn moves
                                                if (!(target_square > h1) && !get_bit(occupancies[both], target_square)){
                                                    // pawn promotion
                                                        if (source_square >= a2 && source_square <= h2){
                                                            add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
                                                            add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
                                                            add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
                                                            add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
                                                        }
                                
                                                        else{

                                                            // one square ahead pawn move
                                                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                                    
                                                            // two squares ahead pawn move
                                                                if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square + 8))
                                                                add_move(move_list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
                                                        }
                                                }
                            
                                            // init pawn attacks bitboard
                                                attacks = pawn_attacks[side][source_square] & occupancies[white];
                            
                                            // generate pawn captures
                                                while (attacks){

                                                    // init target square
                                                        target_square = get_ls1b_index(attacks);
                                
                                                    // pawn promotion
                                                        if (source_square >= a2 && source_square <= h2){
                                                            add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
                                                            add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
                                                            add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
                                                            add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
                                                        }
                                
                                                        else
                                                            // one square ahead pawn move
                                                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                                
                                                            // pop ls1b of the pawn attacks
                                                                pop_bit(attacks, target_square);
                                                }
                            
                                            // generate enpassant captures
                                                if (enpassant != no_sq){

                                                    // lookup pawn attacks and bitwise AND with enpassant square (bit)
                                                        U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                                
                                                    // make sure enpassant capture available
                                                        if (enpassant_attacks){

                                                            // init enpassant capture target square
                                                                int target_enpassant = get_ls1b_index(enpassant_attacks);
                                                                add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                                                        }
                                                }
                            
                                            // pop ls1b from piece bitboard copy
                                                pop_bit(bitboard, source_square);
                                        }
                                }
                    
                            // castling moves
                                if (piece == k){

                                    // king side castling is available
                                        if (castle & bk){

                                            // make sure square between king and king's rook are empty
                                                if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8)){

                                                    // make sure king and the f8 squares are not under attacks
                                                        if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
                                                        add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
                                                }
                                        }
                        
                                    // queen side castling is available
                                        if (castle & bq){
                                            // make sure square between king and queen's rook are empty
                                                if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8)){

                                                    // make sure king and the d8 squares are not under attacks
                                                        if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
                                                            add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
                                                }
                                        }
                                }
                        }
                
                        // genarate knight moves
                            if ((side == white) ? piece == N : piece == n){

                                // loop over source squares of piece bitboard copy
                                    while (bitboard){
                                        // init source square
                                            source_square = get_ls1b_index(bitboard);
                        
                                        // init piece attacks in order to get set of target squares
                                            attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                        
                                        // loop over target squares available from generated attacks
                                            while (attacks){

                                                // init target square
                                                    target_square = get_ls1b_index(attacks);    
                            
                                                // quite move
                                                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                                                    else
                                                        // capture move
                                                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                            
                                                // pop ls1b in current attacks set
                                                    pop_bit(attacks, target_square);
                                            }
                        
                                        // pop ls1b of the current piece bitboard copy
                                            pop_bit(bitboard, source_square);
                                    }
                            }
                
                    // generate bishop moves
                        if ((side == white) ? piece == B : piece == b){

                            // loop over source squares of piece bitboard copy
                                while (bitboard){

                                    // init source square
                                        source_square = get_ls1b_index(bitboard);
                        
                                    // init piece attacks in order to get set of target squares
                                        attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                        
                                    // loop over target squares available from generated attacks
                                        while (attacks){

                                            // init target square
                                                target_square = get_ls1b_index(attacks);    
                            
                                            // quite move
                                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                                                else
                                                    // capture move
                                                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                            
                                            // pop ls1b in current attacks set
                                                pop_bit(attacks, target_square);
                                        }
                        
                        
                                    // pop ls1b of the current piece bitboard copy
                                        pop_bit(bitboard, source_square);
                                }
                        }
                
                    // generate rook moves
                        if ((side == white) ? piece == R : piece == r){

                            // loop over source squares of piece bitboard copy
                                while (bitboard){

                                    // init source square
                                        source_square = get_ls1b_index(bitboard);
                        
                                    // init piece attacks in order to get set of target squares
                                        attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                        
                                    // loop over target squares available from generated attacks
                                        while (attacks){

                                            // init target square
                                                target_square = get_ls1b_index(attacks);    
                            
                                            // quite move
                                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                                                else
                                                    // capture move
                                                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                            
                                                    // pop ls1b in current attacks set
                                                        pop_bit(attacks, target_square);
                                        }
                        
                        
                                    // pop ls1b of the current piece bitboard copy
                                        pop_bit(bitboard, source_square);
                                }
                        }
                
                    // generate queen moves
                        if ((side == white) ? piece == Q : piece == q){

                            // loop over source squares of piece bitboard copy
                                while (bitboard){

                                    // init source square
                                        source_square = get_ls1b_index(bitboard);
                        
                                    // init piece attacks in order to get set of target squares
                                        attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                        
                                    // loop over target squares available from generated attacks
                                        while (attacks){

                                            // init target square
                                                target_square = get_ls1b_index(attacks);    
                            
                                            // quite move
                                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                                                else
                                                    // capture move
                                                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                            
                                                    // pop ls1b in current attacks set
                                                        pop_bit(attacks, target_square);
                                        }           
                        
                        
                                    // pop ls1b of the current piece bitboard copy
                                        pop_bit(bitboard, source_square);
                                }
                        }

                    // generate king moves
                        if ((side == white) ? piece == K : piece == k){

                            // loop over source squares of piece bitboard copy
                                while (bitboard){

                                    // init source square
                                        source_square = get_ls1b_index(bitboard);
                        
                                    // init piece attacks in order to get set of target squares
                                        attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                        
                                    // loop over target squares available from generated attacks
                                        while (attacks){

                                            // init target square
                                                target_square = get_ls1b_index(attacks);    
                            
                                            // quite move
                                                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                                                else
                                                    // capture move
                                                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                            
                                            // pop ls1b in current attacks set
                                                pop_bit(attacks, target_square);
                                        }

                                    // pop ls1b of the current piece bitboard copy
                                        pop_bit(bitboard, source_square);
                                }
                        }
                }
            }

//////////////////////////////////////////////////////////// Perft //////////////////////////////////////////////////////////////////////////////////////

    // leaf nodes (number of positions reached during the test of the move generator at a given depth)
        U64 nodes;

    // perft driver
        static inline void perft_driver(int depth){

            // reccursion escape condition
                if (depth == 0){

                    // increment nodes count (count reached positions)
                        nodes++;
                        return;
                }
    
            // create move list instance
                moves move_list[1];
    
            // generate moves
                generate_moves(move_list);
    
            // loop over generated moves
                for (int move_count = 0; move_count < move_list->count; move_count++){   

                    // preserve board state
                        copy_board();
        
                    // make move
                        if (!make_move(move_list->moves[move_count], all_moves))
                            // skip to the next move
                                continue;
        
                    // call perft driver recursively
                        perft_driver(depth - 1);
        
                    // take back
                        take_back();

                    // debug hask key incremental updates 
                    // build hash key for the updated position(after the move is made) from scratch
                        /*U64 hash_from_scratch = generate_hash_key();

                    /// in case if hash key built from scratch dosen´t match 
                    /// the on that was incrementally update we interrupt execution 
                        if(hash_key != hash_from_scratch){
                                    
                            printf("\n\nTake Back\n");
                            printf("move: "); print_move(move_list->moves[move_count]);
                            print_board();
                            printf("hash key should be: %llx\n", hash_from_scratch);
                            getchar();

                        }*/
                }
        }

    // perft test
        void perft_test(int depth){

            printf("\n     Performance test\n\n");
    
            // create move list instance
                moves move_list[1];
    
            // generate moves
                generate_moves(move_list);
    
            // init start time
                long start = get_time_ms();
    
            // loop over generated moves
                for (int move_count = 0; move_count < move_list->count; move_count++){   

                    // preserve board state
                        copy_board();
        
                    // make move
                        if (!make_move(move_list->moves[move_count], all_moves))
                            // skip to the next move
                                continue;
        
                    // cummulative nodes
                        long cummulative_nodes = nodes;
        
                    // call perft driver recursively
                        perft_driver(depth - 1);
        
                    // old nodes
                        long old_nodes = nodes - cummulative_nodes;
        
                    // take back
                        take_back();
        
                    // print move
                        printf("     move: %s%s%c  nodes: %ld\n", square_to_coordinates[get_move_source(move_list->moves[move_count])],
                                                                  square_to_coordinates[get_move_target(move_list->moves[move_count])],
                                                                  get_move_promoted(move_list->moves[move_count]) ? promoted_pieces[get_move_promoted(move_list->moves[move_count])] : ' ',
                                                                  old_nodes);
                }
    
            // print results
                printf("\n    Depth: %d\n", depth);
                printf("    Nodes: %lld\n", nodes);
                printf("     Time: %ld\n\n", get_time_ms() - start);
        }

//////////////////////////////////////////////////////////// Evaluate //////////////////////////////////////////////////////////////////////////////////////


    // material scrore

    /*
        ♙ =   100   = ♙
        ♘ =   300   = ♙ * 3
        ♗ =   350   = ♙ * 3 + ♙ * 0.5
        ♖ =   500   = ♙ * 5
        ♕ =   1000  = ♙ * 10
        ♔ =   10000 = ♙ * 100
    
    */

   const int material_score[2][12] ={
        // opening material score
            90, 320, 355, 520, 1050, 12000, -90, -320, -355, -520, -1050, -12000,
        
        // endgame material score
            100, 290, 310, 500, 930, 12000, -100, -290, -310, -500, -930, -12000
    };

    // game phase scores
        const int opening_phase_score = 6500;
        const int endgame_phase_score = 450;

    // game phases
        enum { opening, endgame, middlegame };

    // piece types
        enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

    // positional piece scores [game phase][piece][square]
    const int positional_score[2][6][64] ={

        // Abertura: Positional piece scores //

        // Peão
            0,   0,   0,   0,   0,   0,  0,   0,
            90, 120,  70,  100, 80, 130, 40, -10,
            -5,  10,  25,  35,  60,  50, 20, -20,
            -10,  15,  10,  20,  25,  15, 10, -15,
            -20,   5,  -5,  10,  15,   5,  5, -20,
            -25,   0,  -5, -10,   5,   5, 25, -15,
            -30,   5, -15, -20, -10,  20, 35, -20,
            0,   0,   0,   0,   0,   0,  0,   0,

        // Cavalo
            -160, -80, -40, -40,  50, -90, -20, -100,
            -60, -30,  70,  40,  20,  60,  10,  -15,
            -40,  50,  30,  60,  80, 120,  70,   30,
            -10,  20,  20,  50,  40,  70,  15,   20,
            -10,  10,  15,  20,  30,  15,  10,  -10,
            -20,   0,  10,  10,  20,  15,  20,  -20,
            -30, -50,  -5,   0,  10,  15, -10,  -25,
            -100, -20, -60, -30, -20, -25, -20,  -30,

        // Bispo
            -20,  10, -70, -30, -20, -30,  10,  -5,
            -20,  20, -10, -10,  35,  60,  15, -30,
            -10,  40,  40,  40,  30,  45,  40,   5,
            0,   10,  20,  40,  30,  30,  10,   0,
            -10,  10,  15,  20,  30,  15,  15,   5,
            5,   15,  10,  10,  15,  25,  20,  15,
            10,  10,  15,   5,   5,  25,  30,   5,
            -30,   0, -20, -20, -10, -10, -30, -20,

        // Torre
            40,  50,  40,  60, 60, 10,  35,  50,
            30,  40,  60,  65, 80, 60,  30,  50,
            5,  20,  30,  40, 20, 40,  60,  20,
            -10,  -5,  10,  30, 30, 30,   5, -10,
            -20, -10,   5,   0,  15,  5,  10, -20,
            -35, -20, -10, -15,  5,   5,  -5, -25,
            -30, -10, -15,  -5,  0,  10,   0, -60,
            -15,  -5,   5,  20, 15, 10, -20, -15,

        // Rainha
            -20,  10,  30,  15,  60,  50,  40,  50,
            -10, -30,   0,  10, -10,  60,  25,  55,
            0, -10,  10,  20,  35,  55,  50,  55,
            -20, -20, -10, -10,   0,  15,   0,   0,
            -5, -15,   5,   0,   5,   5,  10,   5,
            -10,   5,  -5,   5,  10,  10,  15,  10,
            -25,  -5,  10,   5,  15,  10,   5,  10,
            0, -15, -10,  15, -10, -20, -20, -40,

        // Rei
            -60,  25,  20, -10, -50, -30,   5,  15,
            35,   5, -10,   5,   0,   0, -35, -25,
            0,  25,   5, -10, -10,  10,  30, -15,
            -10, -10,  -5, -20, -25, -20,  -5, -30,
            -40,   5, -20, -30, -40, -40, -20, -45,
            -10, -10, -15, -40, -40, -20,  -5, -20,
            5,  10,  -5, -50, -40, -10,  10,  10,
            -10,  40,  20, -40,  10, -20,  35,  15,

        // Endgame: Positional piece scores //

        // Peão
            0,   0,   0,   0,   0,   0,   0,   0,
            170, 160, 150, 130, 140, 130, 160, 180,
            80,  90,  80,  60,  60,  55,  80,  85,
            20,  20,  10,   0,  -5,   5,  10,  10,
            10,   5,  -5,  -5,  -5, -10,   5,   0,
            0,   5,  -5,   5,   5,  -5,   0, -10,
            10,   5,   5,  10,  10,   5,   5,  -5,
            0,   0,   0,   0,   0,   0,   0,   0,

        // Cavalo
            -50, -30, -10, -20, -20, -25, -60, -90,
            -20,   0, -20,   0,  -5, -20, -20, -50,
            -20, -15,  15,  10,   0, -10, -15, -30,
            -10,   5,  20,  25,  20,  10,  15, -10,
            -10,   0,  20,  25,  15,  15,   5, -15,
            -10,   0,   5,  20,  15,   0, -10, -20,
            -30, -10,  -5,   0,   5, -20, -20, -40,
            -60, -50, -40, -25, -20, -40, -70, -80,

        // Bispo
            10,  10, -15,   5,  -5, -10,   5,   0,
            0,   5,  10,  10,  20,  15,  10,  -5,
            -15,  -5,   5,  15,  20,  15,   5, -10,
            -10,   5,  15,  15,  20,   5,   0, -10,
            -10,   5,   5,  10,  15,  10,   0, -10,
            -20,   0,   5,  15,  10,  15,   5, -10,
            -20, -10, -10,  -5, -10, -20, -10, -10,
            -20, -20, -10, -20, -20, -20, -15, -30,

        // Torre
            20,  20,  20,  25,  20,  20,  15,  10,
            10,  20,  25,  25,  25,  25,  20,   5,
            5,  15,  10,  10,  10,  10,   0,  -5,
            0,  10,   0,   5,   5,   0, -10, -10,
            -10,   5,  10,  10,  -5,   0,   0, -15,
            -20,   0,   5,   0,  -5, -10,  -5, -10,
            -10,   0,   0, -10, -10, -10, -10, -10,
            -10,  -5,   0,   5,   5,  -5,  -5, -20,

        // Rainha
            -15,  -5,  10,  -5,  15,  15,  -5, -10,
            -10,   0,   5,  10,   5,  10,   0,  -5,
            -5,  -5,  -5,   5,  -5,  -5,   5, -15,
            -5,   0,   0,   0,   0,  -5,  -5,  -5,
            0,   5,   5,  -5,   0,   0,   0, -10,
            -5,  -5,   0,   0,  -5,  -5,   0, -10,
            -5,   0,   5,  10,   5,  10,   0, -10,
            -15,  -5,  10,  -5,  15,  15,  -5, -10,

        // Rei
            40,  50,  20,  15,   5,   0,  50,  40,
            40,  40,  10, -10,  -5, -20,  30,  30,
            20,  10, -10, -20, -30, -40,  10,  20,
            5,   0, -20, -25, -35, -30,   0,  10,
            -10, -10, -25, -35, -45, -40, -10,   5,
            -20, -20, -35, -50, -50, -40, -15,   0,
            -30, -30, -50, -50, -60, -55, -20, -10,
            -30, -20, -20, -10, -15, -25, -25, -20
    };

    // mirror positional score tables for opposite side
        const int mirror_score[128] =
        {
            a1, b1, c1, d1, e1, f1, g1, h1,
            a2, b2, c2, d2, e2, f2, g2, h2,
            a3, b3, c3, d3, e3, f3, g3, h3,
            a4, b4, c4, d4, e4, f4, g4, h4,
            a5, b5, c5, d5, e5, f5, g5, h5,
            a6, b6, c6, d6, e6, f6, g6, h6,
            a7, b7, c7, d7, e7, f7, g7, h7,
            a8, b8, c8, d8, e8, f8, g8, h8
        };


    // file masks [square]
        U64 file_masks[64];

    // rank masks [square]
        U64 rank_masks[64];

    // isolated pawn masks [square]
        U64 isolated_masks[64];

    // passed pawns masks [square]
        U64 white_passed_masks[64];

    // passed pawns masks [square]
        U64 black_passed_masks[64];

    // extract rank from a square [square]
        const int get_rank[64] =
        {
            7, 7, 7, 7, 7, 7, 7, 7,
            6, 6, 6, 6, 6, 6, 6, 6,
            5, 5, 5, 5, 5, 5, 5, 5,
            4, 4, 4, 4, 4, 4, 4, 4,
            3, 3, 3, 3, 3, 3, 3, 3,
            2, 2, 2, 2, 2, 2, 2, 2,
            1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0
        };

    // double pawns penalty
        const int double_pawn_penalty = -10;

    // isolated pawn penalty
        const int isolated_pawn_penalty = -10;

    // // passed pawn bonus
        const int passed_pawn_bonus[8] = { 0, 10, 20, 40, 70, 110, 160, 250 };

    // semi-open and open files score
        const int semi_open_file_score = 10;
        const int open_file_score = 20;

    // king safety bonus  
        const int king_safety_bonus = 15;

    // set file or rank mask
        U64 set_file_rank_mask(int file_number, int rank_number){

            // file or rank mask
                U64 mask = 0ULL;
            
            // loop over files and ranks
                for ( int rank = 0; rank < 8; rank++){

                // loop over files
                    for ( int file = 0; file < 8; file++){
                        
                    // init square
                        int square = rank * 8 + file;

                    // on file match
                        if(file_number != -1) {
                            if ( file == file_number)
                            // set bit on mask 
                            mask |= set_bit(mask, square);
                            
                        }

                    // on rank match
                        else if(rank_number != -1){
                            if ( rank == rank_number)
                            // set bit on mask 
                            mask |= set_bit(mask, square);
                        }
                    }
                    
                }
            
            // return mask
                return mask;

        }

// init evaluation masks
    void init_evaluation_masks(){

            /********** INIT FILE MASKS **********/

        // loop over files and ranks
            for ( int rank = 0; rank < 8; rank++){

            // loop over files
                for ( int file = 0; file < 8; file++){
                    // init square
                        int square = rank * 8 + file;

                    // init file mask for current square
                        file_masks[square] |= set_file_rank_mask(file, -1);
                }
            }

            /********** INIT RANK MASKS **********/
            
        // loop over files and ranks
            for ( int rank = 0; rank < 8; rank++){

            // loop over files
                for ( int file = 0; file < 8; file++){
                    // init square
                        int square = rank * 8 + file;

                    // init file mask for current square
                        rank_masks[square] |= set_file_rank_mask(-1, rank);
                }
            }

            /********** INIT ISOLATED MASKS **********/
            
        // loop over files and ranks
            for ( int rank = 0; rank < 8; rank++){

            // loop over files
                for ( int file = 0; file < 8; file++){
                    // init square
                        int square = rank * 8 + file;

                    // init file mask for current square
                        isolated_masks[square] |= set_file_rank_mask(file - 1, -1);
                        isolated_masks[square] |= set_file_rank_mask(file + 1, -1);
                }
            }

        // loop over files and ranks
            for ( int rank = 0; rank < 8; rank++){

            // loop over files
                for ( int file = 0; file < 8; file++){
                    // init square
                        int square = rank * 8 + file;

                    // init file mask for current square
                        isolated_masks[square] |= set_file_rank_mask(file - 1, -1);
                        isolated_masks[square] |= set_file_rank_mask(file + 1, -1);
                }
            }

        /********** WHITE PASSED MASKS **********/

        // loop over files and ranks
            for ( int rank = 0; rank < 8; rank++){

            // loop over files
                for ( int file = 0; file < 8; file++){
                    // init square
                        int square = rank * 8 + file;

                    // init file mask for current square
                        white_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
                        white_passed_masks[square] |= set_file_rank_mask(file, -1);
                        white_passed_masks[square] |= set_file_rank_mask(file + 1, -1);

                        for (int i = 0; i < (8 -rank); i++)
                        {
                            // reset redudant bits
                                white_passed_masks[square] &= ~rank_masks[(7 - i) * 8 + file];
                        }
                }
            }

        /********** BLACK PASSED MASKS **********/

        // loop over files and ranks
            for ( int rank = 0; rank < 8; rank++){

            // loop over files
                for ( int file = 0; file < 8; file++){
                    // init square
                        int square = rank * 8 + file;

                    // init file mask for current square
                        black_passed_masks[square] |= set_file_rank_mask(file - 1, -1);
                        black_passed_masks[square] |= set_file_rank_mask(file, -1);
                        black_passed_masks[square] |= set_file_rank_mask(file + 1, -1);

                        for (int i = 0; i < rank + 1; i++)
                        {
                            // reset redudant bits
                                black_passed_masks[square] &= ~rank_masks[i * 8 + file];
                        }
                }
            }
    }
                    
// get game phase
    static inline int get_game_phase_score(){

        // white & black game phase scores
            int white_piece_scores = 0, black_piece_scores = 0;
        
        // loop over white pieces
            for (int piece = N; piece <= Q; piece++)
                white_piece_scores += count_bits(bitboards[piece]) * material_score[opening][piece];
            
        // loop over white pieces
            for (int piece = n; piece <= q; piece++)
                black_piece_scores += count_bits(bitboards[piece]) * -material_score[opening][piece];
            
        // return game phase score
            return white_piece_scores + black_piece_scores;

        }
        
        
    // position evaluation
    static inline int evaluate() {
        int game_phase_score = get_game_phase_score();
        int game_phase = (game_phase_score > opening_phase_score) ? opening :
                         (game_phase_score < endgame_phase_score) ? endgame : middlegame;
    
        int score = 0, score_opening = 0, score_endgame = 0;
        U64 bitboard;
        int piece, square;
    
        for (int bb_piece = P; bb_piece <= k; bb_piece++) {
            bitboard = bitboards[bb_piece];
            while (bitboard) {
                piece = bb_piece;
                square = get_ls1b_index(bitboard);
                score_opening += material_score[opening][piece];
                score_endgame += material_score[endgame][piece];
    
                switch (piece) {
                    case P: case p:
                        score_opening += positional_score[opening][PAWN][square];
                        score_endgame += positional_score[endgame][PAWN][square];
                        break;
                    case N: case n:
                        score_opening += positional_score[opening][KNIGHT][square];
                        score_endgame += positional_score[endgame][KNIGHT][square];
                        break;
                    case B: case b:
                        score_opening += positional_score[opening][BISHOP][square];
                        score_endgame += positional_score[endgame][BISHOP][square];
                        break;
                    case R: case r:
                        score_opening += positional_score[opening][ROOK][square];
                        score_endgame += positional_score[endgame][ROOK][square];
                        break;
                    case Q: case q:
                        score_opening += positional_score[opening][QUEEN][square];
                        score_endgame += positional_score[endgame][QUEEN][square];
                        break;
                    case K: case k:
                        score_opening += positional_score[opening][KING][square];
                        score_endgame += positional_score[endgame][KING][square];
                        break;
                }
                pop_bit(bitboard, square);
            }
        }
    
        if (game_phase == middlegame)
            score = (score_opening * game_phase_score + score_endgame * (opening_phase_score - game_phase_score)) / opening_phase_score;
        else if (game_phase == opening) score = score_opening;
        else if (game_phase == endgame) score = score_endgame;
    
        return (side == white) ? score : -score;
    }

//////////////////////////////////////////////////////////// Search //////////////////////////////////////////////////////////////////////////////////////
    
    // score bounds for the mating scores
        #define infinity 50000
        #define mate_value 49000
        #define mate_score 48000

    // most valuable victim & less valuable attacker
    /*
                            
        (Victims) Pawn Knight Bishop   Rook  Queen   King
    (Attackers)
            Pawn   105    205    305    405    505    605
        Knight   104    204    304    404    504    604
        Bishop   103    203    303    403    503    603
            Rook   102    202    302    402    502    602
        Queen   101    201    301    401    501    601
            King   100    200    300    400    500    600

    */

    // MVV LVA [attacker][victim]
        static int mvv_lva[12][12] = {
            105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
            104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
            103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
            102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
            101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
            100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

            105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
            104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
            103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
            102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
            101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
            100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
        };

    // max ply that we can reach within a search
        #define max_ply 64

    // killer moves [id][ply]
        int killer_moves[2][max_ply];

    // history moves [piece][square]
        int history_moves[12][64];

    /*
        ================================
                Triangular PV table
        --------------------------------
            PV line: e2e4 e7e5 g1f3 b8c6
        ================================

            0    1    2    3    4    5
        
        0    m1   m2   m3   m4   m5   m6
        
        1    0    m2   m3   m4   m5   m6 
        
        2    0    0    m3   m4   m5   m6
        
        3    0    0    0    m4   m5   m6
        
        4    0    0    0    0    m5   m6
        
        5    0    0    0    0    0    m6
    */

    // PV length [ply]
        int pv_length[max_ply];

    // PV table [ply][ply]
        int pv_table[max_ply][max_ply];

    // follow PV & score PV move
        int follow_pv, score_pv;

//////////////////////////////////////////////////////////// Transposition Tables //////////////////////////////////////////////////////////////////////////////////////

    // number hash table entries
        int hash_entries = 0;

    // no hash entry found constant
        #define no_hash_entry 100000

    // transposition table hash flags
        #define hash_flag_exact 0
        #define hash_flag_alpha 1
        #define hash_flag_beta 2

    // transposition table data structure
        typedef struct {
            U64 hash_key;   // "almost" unique chess position identifier
            int depth;      // current search depth
            int flag;       // flag the type of node (fail-low/fail-high/PV) 
            int score;      // score (alpha/beta/PV)
        } tt;               // transposition table (TT aka hash table)

    // define TT instance
        tt *hash_table = NULL;

    // clear TT (hash table)
    void clear_hash_table()
    {
        // init hash table entry pointer
            tt *hash_entry;

        // loop over TT elements
            for (hash_entry = hash_table; hash_entry < hash_table + hash_entries; hash_entry++){
                // reset TT inner fields
                hash_entry->hash_key = 0;
                hash_entry->depth = 0;
                hash_entry->flag = 0;
                hash_entry->score = 0;
            }
    }

    // dynamically allocate memory for hash table
        void init_hash_table(int mb){
            // init hash size
                int hash_size = 0x100000 * mb;
            
            // init number of hash entries
                hash_entries =  hash_size / sizeof(tt);

            // free hash table if not empty
                if (hash_table != NULL){
                    printf("    Clearing hash memory...\n");
                    
                    // free hash table dynamic memory
                    free(hash_table);
                }
            
            // allocate memory
                hash_table = (tt *) malloc(hash_entries * sizeof(tt));

            // if allocation has failed
                if (hash_table == NULL){
                    printf("    Couldn't allocate memory for hash table, tryinr %dMB...", mb / 2);
                    
                    // try to allocate with half size
                    init_hash_table(mb / 2);
                }
            
            // if allocation succeeded
                else{
                    // clear hash table
                    clear_hash_table();
                    
                    printf("    Hash table is initialied with %d entries\n", hash_entries);
                }    
        }

    // read hash entry data
        static inline int read_hash_entry(int alpha, int beta, int depth){

            /// create a TT instance pointer to particular hash entry storing the 
            /// scoring data for the current board position if available
                tt *hash_entry = &hash_table[hash_key % hash_entries];

            // make sure we´re dealing with the same exact position 
                if (hash_entry->hash_key == hash_key){

                // make sure that we have the same depth as the search 
                    if (hash_entry->depth >= depth) {
                        
                        // extract stored score from TT entry
                            int score = hash_entry->score;

                        // retrieve score independent from the actual path from root position to current node
                            if (score < -mate_score) score += ply;
                            if (score > mate_score) score -= ply;

                        // match the exact (PV node) score    
                            if (hash_entry->flag == hash_flag_exact)
                                return score; // return exact (PV node) score 

                        // match alpha (fail-low node) score
                            if ((hash_entry->flag == hash_flag_alpha) && (score <= alpha))
                               return alpha; // return alpha (fail-low node) score

                        // match beta (fail high node) score
                            if ((hash_entry->flag == hash_flag_beta) &&(score  >= beta))
                                return beta; // return beta (fail-high node) score

                    }
                }

            // if hash entry dosent exit
                return no_hash_entry;
        }

    // write hash entry data
        static inline void write_hash_entry(int score, int depth, int hash_flag){

            /// create a TT instance pointer to particular hash entry storing the 
            /// scoring data for the current board position if available
                tt *hash_entry = &hash_table[hash_key % hash_entries];

            // store score independent from the actual path from root position to current node
                if (score <- mate_score) score -= ply;
                if (score > mate_score) score += ply;

            // write hash entry data
                hash_entry->hash_key = hash_key;
                hash_entry->score = score;
                hash_entry->flag = hash_flag;
                hash_entry->depth = depth;
        }

    // enable PV move scoring
        static inline void enable_pv_scoring(moves *move_list){

            // disable following PV
                follow_pv = 0;
            
            // loop over the moves within a move list
                for (int count = 0; count < move_list->count; count++){

                    // make sure we hit PV move
                        if (pv_table[0][ply] == move_list->moves[count]){
                            // enable move scoring
                                score_pv = 1;
                    
                            // enable following PV
                                follow_pv = 1;
                        }
                }
        }

    // Sort moves based on MVV-LVA, killer moves, and history heuristic
        static inline int score_move(int move) {
            if (score_pv && pv_table[0][ply] == move) {
                score_pv = 0;
                return 20000; // Highest priority for PV move
            }
            if (get_move_capture(move)) {
                int target_piece = P;
                for (int bb_piece = p; bb_piece <= k; bb_piece++) {
                    if (get_bit(bitboards[bb_piece], get_move_target(move))) {
                        target_piece = bb_piece;
                        break;
                    }
                }
                return mvv_lva[get_move_piece(move)][target_piece] + 10000; // Prioritize captures
            } else {
                if (killer_moves[0][ply] == move) return 9000; // Killer move 1
                if (killer_moves[1][ply] == move) return 8000; // Killer move 2
                return history_moves[get_move_piece(move)][get_move_target(move)]; // History heuristic
            }
        }

    // sort moves in descending order
        static inline int sort_moves(moves *move_list){

            // move scores
                int move_scores[move_list->count];
        
            // score all the moves within a move list
                for (int count = 0; count < move_list->count; count++)
                    // score move
                        move_scores[count] = score_move(move_list->moves[count]);
        
            // loop over current move within a move list
                for (int current_move = 0; current_move < move_list->count; current_move++){

                    // loop over next move within a move list
                        for (int next_move = current_move + 1; next_move < move_list->count; next_move++){

                            // compare current and next move scores
                                if (move_scores[current_move] < move_scores[next_move]){

                                    // swap scores
                                        int temp_score = move_scores[current_move];
                                        move_scores[current_move] = move_scores[next_move];
                                        move_scores[next_move] = temp_score;
                    
                                    // swap moves
                                        int temp_move = move_list->moves[current_move];
                                        move_list->moves[current_move] = move_list->moves[next_move];
                                        move_list->moves[next_move] = temp_move;
                                }
                        }
                }
        }

    // print move scores
        void print_move_scores(moves *move_list){

            printf("     Move scores:\n\n");
            
            // loop over moves within a move list
                for (int count = 0; count < move_list->count; count++){
                    printf("     move: ");
                    print_move(move_list->moves[count]);
                    printf(" score: %d\n", score_move(move_list->moves[count]));
                }
        }

    // position repetition detection
        static inline int is_repetition(){

            // loop over repetition indicies range
            for (int index = 0; index < repetition_index; index++)                                    
                if (repetition_table[index] == hash_key) // if we found the hash key same with a current
                    return 1; // we found a repetition
            
            // if no repetition found
            return 0;
        }

    // quiescence search
        static inline int quiescence(int alpha, int beta){

            // every 2047 nodes
                if((nodes & 2047 ) == 0)
                    // "listen" to the GUI/user input
		                communicate();

            // increment nodes count
                nodes++;

            // we are too deep (prevent overflow in arrays)
                if (ply > max_ply - 1)
                 // evaluate position
                    return evaluate();

            // evaluate position
                int evaluation = evaluate();

            // fail-hard beta cutoff
                if (evaluation >= beta){
                    // node (move) fails high
                        return beta;
                }
        
            // found a better move
                if (evaluation > alpha){
                    // PV node (move)
                        alpha = evaluation;
                }
        
            // create move list instance
                moves move_list[1];
        
            // generate moves
                generate_moves(move_list);
        
            // sort moves
                sort_moves(move_list);
        
            // loop over moves within the movelist
                for (int count = 0; count < move_list->count; count++){

                    // preserve board state
                        copy_board();
            
                    // increment ply
                        ply++;

                    // increment repetion index and store hash key 
                        repetition_index++;
                        repetition_table[repetition_index] = hash_key;
                
                    // make sure to make only legal moves
                        if (make_move(move_list->moves[count], only_captures) == 0){

                            // decrement ply
                                ply--;

                            // decrement repetition index 
                                repetition_index--;

                            // skip to next move
                                continue;
                        }

                    // score current move
                        int score = -quiescence(-beta, -alpha);
            
                    // decrement ply
                        ply--;

                    // decrement repetition index 
                        repetition_index--;

                    // take move back
                        take_back();

                     // return 0 if time is up
                        if(stopped == 1) return 0;

                    // found a better move
                        if (score > alpha){

                            // PV node (move)
                                alpha = score; 

                             // fail-hard beta cutoff
                                if (score >= beta){
                                    return beta; // node (move) fails high
                                }
                        }

                }
        
            // node (move) fails low
                return alpha;
        }

    const int full_depth_moves = 4;
    const int reduction_limit = 3;

    //  alpha beta search
        static inline int negamax(int alpha, int beta, int depth){

            // variable to score move´s score
                int score;
            
            // define hash flag
                int hash_flag = hash_flag_alpha;

            // if position repetition appears
                if(ply && is_repetition())
                    return 0; // return draw score 

            // a hack to figure out whether the current node is PV node or not 
                int pv_node = (beta - alpha > 1);

            // read hash entry we´re not in a root ply and hash entry is available and current node is not a PV node
                if ( ply && (score = read_hash_entry(alpha, beta, depth)) != no_hash_entry && !pv_node)
                    // if the move has already searched, we just return the score, without research it 
                        return score;

            // every 2047 nodes
                if((nodes & 2047 ) == 0)
                    // "listen" to the GUI/user input
		                communicate();

            // init PV length
                pv_length[ply] = ply;

            // recursion escapre conditionnegamax
                if (depth == 0)
                    // run quiescence search
                        return quiescence(alpha, beta);
        
            // we are too deep
                if (ply > max_ply - 1)
                 // evaluate position
                    return evaluate();
        
            // increment nodes count
                nodes++;
        
            // is king in check
                int in_check = is_square_attacked((side == white) ? get_ls1b_index(bitboards[K]) : 
                                                                    get_ls1b_index(bitboards[k]),
                                                                    side ^ 1);
        
            // increase search depth if the king has been exposed into a check
                if (in_check) depth++;
        
            // legal moves counter
                int legal_moves = 0;

            // null move pruning
                if(depth >= 3 && in_check == 0 && ply){

                    // preserve board state
                        copy_board();

                    // increment ply
                        ply++;

                    // increment repetion index and store hash key 
                        repetition_index++;
                        repetition_table[repetition_index] = hash_key;
                    
                    // hash enpassant if avalable
                        if(enpassant != no_sq) hash_key ^= enpassant_keys[enpassant];

                    // reset enpassant capture square   
                        enpassant = no_sq;
                    
                    // switch the side, give a opponent to make an extra move 
                        side ^= 1;

                    // hash the side    
                        hash_key ^= side_key;
                    
                    /// search move with reduced depth to find beta cutoffs
                    /// (depth - 1 - R), R is a reduction limit 
                        score = -negamax(-beta, -beta + 1, depth - 1 - 2);

                    // decrement ply
                        ply--;

                    // decrement repetition index 
                        repetition_index--;

                    // restore board state
                        take_back();

                    // return 0 if time is up
                        if(stopped == 1) return 0;

                    // fail-hard beta cutoff
                        if (score >= beta)
                            // node fails high
                                return beta;

                }
        
            // create move list instance
                moves move_list[1];
        
            // generate moves
                generate_moves(move_list);
        
            // if we are now following PV line
                if (follow_pv)
                    // enable PV move scoring
                        enable_pv_scoring(move_list);
        
            // sort moves
                sort_moves(move_list);

            // number of moves searched
                int moves_searched = 0;
        
            // loop over moves within a movelist
            for (int count = 0; count < move_list->count; count++){

                // preserve board state
                    copy_board();
            
                // increment ply
                    ply++;
                
                // increment repetion index and store hash key 
                    repetition_index++;
                    repetition_table[repetition_index] = hash_key;

                // make sure to make only legal moves
                    if (make_move(move_list->moves[count], all_moves) == 0){

                        // decrement ply
                            ply--;

                        // decrement repetition index 
                            repetition_index--;
                
                        // skip to next move
                            continue;
                    }
            
                // increment legal moves
                    legal_moves++;
 
                // full depth search    
                    if(moves_searched == 0 )
                        // do normal alpha beta search
                            score = -negamax(-beta, -alpha, depth - 1);
                                    
                // Late Move Reduction
                    else{

                        // condition to consider Late Move Reduction
                            if(moves_searched >= full_depth_moves && depth >= reduction_limit && in_check == 0 && get_move_capture(move_list->moves[count]) == 0 && get_move_promoted(move_list->moves[count]) == 0)
                                // search current move with reduced depth:
                                    score = -negamax(-alpha - 1 , -alpha, depth-2);

                                // hack to ensure that full-depth search is done.
                                    else score = alpha+1;  
                                    
                                // principal variation search PVS
                                    if(score > alpha) {
                                        // When the engine founds a move with a score between alpha and bet, the rest of the moves are consider bad.
                                            score = -negamax(-alpha - 1, -alpha, depth-1);

                                        // If the algorithm thinks he is wrong, and of the moves was better then the first PV. it has to research
                                            if(score > alpha && score < beta)
                                                // re-search the move
                                                    score = -negamax(-beta, -alpha, depth-1);
                                }         
                        }
                
            
                // decrement ply
                    ply--;

                // decrement repetition index 
                    repetition_index--;

                // take move back
                    take_back();
                     
                // increment the number of moves searched
                    moves_searched++;
            
                // found a better move
                if (score > alpha){

                    // switch hash flag from storing score for fail-low node to the one score for PV node
                        hash_flag = hash_flag_exact;

                    // on quiet moves
                        if (get_move_capture(move_list->moves[count]) == 0)
                            // store history moves
                                history_moves[get_move_piece(move_list->moves[count])][get_move_target(move_list->moves[count])] += depth;
                
                            // PV node (move)
                                alpha = score;

                            // write PV move
                                pv_table[ply][ply] = move_list->moves[count];
                
                            // loop over the next ply
                                for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
                                    // copy move from deeper ply into a current ply's line
                                        pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
                
                            // adjust PV length
                                pv_length[ply] = pv_length[ply + 1];   

                            // fail-hard beta cutoff
                                if (score >= beta){

                                    // store hash entry with the score equal to the beta
                                        write_hash_entry(beta, depth, hash_flag_beta);

                                    // on quiet moves
                                        if (get_move_capture(move_list->moves[count]) == 0){

                                            // store killer moves
                                                killer_moves[1][ply] = killer_moves[0][ply];
                                                killer_moves[0][ply] = move_list->moves[count];
                                        }
                            
                                    // node (move) fails high
                                        return beta;
                                }         
                    }
            }
        
            // we don't have any legal moves to make in the current postion
                if (legal_moves == 0){

                    // king is in check
                        if (in_check)
                            // return mating score 
                                return -mate_value + ply;
            
                    // king is not in check
                        else
                            // return stalemate score
                                return 0;
                }

            // store hash entry with the score equal to the alpha
                write_hash_entry(alpha, depth, hash_flag);

            // node (move) fails low
                return alpha;
        }

    // search position for the best move
        void search_position(int depth){

            // define best score variable
                int score = 0;
        
            // reset nodes counter
                nodes = 0;

            // reset "time is up" flag
                stopped = 0;
        
            // reset follow PV flags
                follow_pv = 0;
                score_pv = 0;
        
            // clear helper data structures for search
                memset(killer_moves, 0, sizeof(killer_moves));
                memset(history_moves, 0, sizeof(history_moves));
                memset(pv_table, 0, sizeof(pv_table));
                memset(pv_length, 0, sizeof(pv_length));

            // define alpha and beta
                int alpha = -infinity;
                int beta = infinity;
        
            // iterative deepening
                for (int current_depth = 1; current_depth <= depth; current_depth++){

                    // if time is up
                        if(stopped == 1)
			                // stop calculating and return best move so far 
			                    break;

                    // enable follow PV flag
                        follow_pv = 1;
            
                    // find best move within a given position
                        score = negamax(alpha, beta, current_depth);

                    // We fell outside the window, so try again with a full-width window (and the same depth).
                        if ((score <= alpha) || (score >= beta)) {
                            alpha = -infinity;    
                            beta = infinity;      
                            continue;
                        }

                    // Set up the window for the next iteration.
                        alpha = score - 50;  
                        beta = score + 50;

                    if (pv_length[0]){

                    if (score > -mate_value && score < -mate_score)
                        printf("info score mate %d depth %d nodes %lld time %d pv ", -(score + mate_value) / 2 - 1, current_depth, nodes, get_time_ms() - starttime);
        
                    else if (score > mate_score && score < mate_value)
                        printf("info score mate %d depth %d nodes %lld time %d pv ", (mate_value - score) / 2 + 1, current_depth, nodes, get_time_ms() - starttime);   
        
                    else
                        printf("info score cp %d depth %d nodes %lld time %d pv ", score, current_depth, nodes, get_time_ms() - starttime);
        
            
                    // loop over the moves within a PV line
                        for (int count = 0; count < pv_length[0]; count++){

                            // print PV move
                                print_move(pv_table[0][count]);
                                printf(" ");
                        }
            
                    // print new line
                        printf("\n");
                }
            }
            // best move placeholder
                printf("bestmove ");
                print_move(pv_table[0][0]);
                printf("\n");
        }


//////////////////////////////////////////////////////////// UCI //////////////////////////////////////////////////////////////////////////////////////

    // parse user/GUI move string input (e.g. "e7e8q")
        int parse_move(char *move_string){

            // create move list instance
                moves move_list[1];
    
            // generate moves
                generate_moves(move_list);
    
            // parse source square
                int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    
            // parse target square
                int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;
    
            // loop over the moves within a move list
                for (int move_count = 0; move_count < move_list->count; move_count++){

                    // init move
                        int move = move_list->moves[move_count];
        
                    // make sure source & target squares are available within the generated move
                        if (source_square == get_move_source(move) && target_square == get_move_target(move)){

                            // init promoted piece
                                int promoted_piece = get_move_promoted(move);
            
                            // promoted piece is available
                                if (promoted_piece){

                                    // promoted to queen
                                        if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
                                            // return legal move
                                                return move;
                
                                    // promoted to rook
                                        else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
                                            // return legal move
                                                return move;
                
                                    // promoted to bishop
                                        else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
                                            // return legal move
                                                return move;
                
                                    // promoted to knight
                                        else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
                                            // return legal move
                                                return move;
                
                                    // continue the loop on possible wrong promotions (e.g. "e7e8f")
                                        continue;
                                }
            
                            // return legal move
                                return move;
                        }
                    }
    
            // return illegal move
                return 0;
        }

    /*
        Example UCI commands to init position on chess board
            
        // init start position
        position startpos
            
        // init start position and make the moves on chess board
        position startpos moves e2e4 e7e5
            
        // init position from FEN string
        position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 
            
        // init position from fen string and make moves on chess board
        position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6 e8g8
    */

    // parse UCI "position" command
        void parse_position(char *command){

            // shift pointer to the right where next token begins
                command += 9;
    
            // init pointer to the current character in the command string
                char *current_char = command;
    
            // parse UCI "startpos" command
                if (strncmp(command, "startpos", 8) == 0)
                    // init chess board with start position
                        parse_fen(start_position);
    
            // parse UCI "fen" command 
                else{

                    // make sure "fen" command is available within command string
                        current_char = strstr(command, "fen");
        
                    // if no "fen" command is available within command string
                        if (current_char == NULL)
                            // init chess board with start position
                                parse_fen(start_position);
            
                    // found "fen" substring
                        else{

                            // shift pointer to the right where next token begins
                                current_char += 4;
            
                            // init chess board with position from FEN string
                                parse_fen(current_char);
                        }
                }
    
            // parse moves after position
                current_char = strstr(command, "moves");
    
            // moves available
                if (current_char != NULL){

                    // shift pointer to the right where next token begins
                        current_char += 6;
        
                    // loop over moves within a move string
                        while(*current_char){

                            // parse next move
                                int move = parse_move(current_char);
            
                            // if no more moves
                                if (move == 0)
                                    // break out of the loop
                                        break;

                            // increment repetition index 
                                repetition_index++;

                            // write hash key into a repetition table
                                repetition_table[repetition_index] = hash_key;
            
                            // make move on the chess board
                                make_move(move, all_moves);
            
                            // move current character mointer to the end of current move
                                while (*current_char && *current_char != ' ') current_char++;
            
                            // go to the next move
                                current_char++;
                        }
                }
        }

    // reset time control variables
        void reset_time_control()
        {
            // reset timing
            quit = 0;
            movestogo = 30;
            movetime = -1;
            time = -1;
            inc = 0;
            starttime = 0;
            stoptime = 0;
            timeset = 0;
            stopped = 0;
        }

    // parse UCI "go" command
        void parse_go(char *command){

            // reset time control
                reset_time_control();

            // init parameters
                int depth = -1;

            // init argument
                char *argument = NULL;

            // infinite search
                if ((argument = strstr(command,"infinite"))) {}

            // match UCI "binc" command
                if ((argument = strstr(command,"binc")) && side == black)
                    // parse black time increment
                        inc = atoi(argument + 5);

            // match UCI "winc" command
                if ((argument = strstr(command,"winc")) && side == white)
                    // parse white time increment
                        inc = atoi(argument + 5);

            // match UCI "wtime" command
                if ((argument = strstr(command,"wtime")) && side == white)
                    // parse white time limit
                        time = atoi(argument + 6);

            // match UCI "btime" command
                if ((argument = strstr(command,"btime")) && side == black)
                    // parse black time limit
                        time = atoi(argument + 6);

            // match UCI "movestogo" command
                if ((argument = strstr(command,"movestogo")))
                    // parse number of moves to go
                        movestogo = atoi(argument + 10);

            // match UCI "movetime" command
                if ((argument = strstr(command,"movetime")))
                    // parse amount of time allowed to spend to make a move
                        movetime = atoi(argument + 9);

            // match UCI "depth" command
                if ((argument = strstr(command,"depth")))
                    // parse search depth
                        depth = atoi(argument + 6);

            // if move time is not available
                if(movetime != -1){

                    // set time equal to move time
                        time = movetime;

                    // set moves to go to 1
                        movestogo = 1;
                }

            // init start time
                starttime = get_time_ms();

            // init search depth
                depth = depth;

            // if time control is available
                if(time != -1){
                    // flag we're playing with time control
                    timeset = 1;

                    // set up timing
                    time /= movestogo;
                    
                    // "illegal" (empty) move fun fix
                    if ( time > 1500 ) time -= 50;

                    // init stoptime
                    stoptime = starttime + time + inc;
                }

            // if depth is not available
                if(depth == -1)
                    // set depth to 64 plies (takes ages to complete...)
                        depth = 64;

            // print debug info
                printf("time:%d start:%u stop:%u depth:%d timeset:%d\n",
                time, starttime, stoptime, depth, timeset);

            // search position
                search_position(depth);
        }


    /*
        GUI -> isready
        Engine -> readyok
        GUI -> ucinewgame
    */

    // main UCI loop
        void uci_loop(){

            // max hash MB 
                int max_hash = 128;
            
            // default MB value 
                int mb = 64;

            // reset STDIN & STDOUT buffers
                setbuf(stdin, NULL);
                setbuf(stdout, NULL);
                
            // define a user / GUI input buffer
                char input[2000];
                
            // print engine info
                printf("id name Aurora\n");
                printf("id author Afonso\n");
                printf("option name Hash type spin default 64 min 4 max %d\n", max_hash);
                printf("uciok\n");

            // main loop
                while (1){

                    // reset user / GUI input
                        memset(input, 0, sizeof(input));

                    // make sure output reaches the GUI
                        fflush(stdout);

                    // get user / GUI input
                        if(!fgets(input, 2000, stdin))
                            // continue the loop
                                continue;
                        
                            // make sure input is available
                                if(input[0] == '\n')
                                    //continue the loop
                                        continue;
                            // parse UCI "isready" command
                                if(strncmp(input, "isready", 7) == 0){

                                    printf("readyok\n");
                                    continue;
                            }
                        
                        // parse UCI "position" command
                            else if(strncmp(input, "position", 8) == 0){

                                // call parse position function
                                    parse_position(input);

                                // clear hash table 
                                    clear_hash_table();
                            }
                        // UCI new game command
                            else if(strncmp(input, " ucinewgame", 10) == 0){
                                // call parse position function
                                    parse_position("position startpos");

                                // clear hash table 
                                    clear_hash_table();
                            }
                        // parse the UCI "go" command
                            else if(strncmp(input, "go", 2) == 0)
                                // call parse position function
                                    parse_go(input);

                        // parse UCI "quit" command
                            else if(strncmp(input, "quit", 4) == 0)
                                // quit from the chess engine program exec
                                    break;

                        // parse UCI "command"
                            else if(strncmp(input, "uci", 3) == 0){

                                // print engine info
                                    printf("id name Aurora\n");
                                    printf("id author Afonso Pires\n");
                                    printf("uciok\n");
                        }

                        else if (!strncmp(input, "setoption name Hash value ", 26)) {			
                            // init MB
                                sscanf(input,"%*s %*s %*s %*s %d", &mb);
                            
                            // adjust MB if going beyond the aloowed bounds
                                if(mb < 4) mb = 4;
                                if(mb > max_hash) mb = max_hash;
                            
                            // set hash table size in MB
                                printf("    Set hash table size to %dMB\n", mb);
                                init_hash_table(mb);
                        }           
                }              
        }

//////////////////////////////////////////////////////////// Main //////////////////////////////////////////////////////////////////////////////////////

    // init all variables
        void init_all(){

            // init leaper pieces attacks
                init_leapers_attacks();
    
            // init slider pieces attacks
                init_sliders_attacks(bishop);
                init_sliders_attacks(rook);

            // init random keys for hasing
                init_random_keys();

            // init evaluation_masks
                init_evaluation_masks();

            // init hash table with default 12 MB
                init_hash_table(12);    
        }
    
    int main(){
    
        // init all
            init_all();
            
        // connect to GUI
            uci_loop();

        // free hash table memory on exit
            free(hash_table);
            
         return 0;
    }


