#pragma once

// #include "cell.h"
#include "steps_representation.h"
#include <array>
#include <Graph_lib/Graph.h>
#include <Graph_lib/Simple_window.h>

constexpr int a_ascii = 'a';

constexpr int chess_yellow = 133;

struct Cell;
struct Chessboard;
struct Coordinate;
struct VisualSteps;

struct Figure : Graph_lib::Image
{

    enum Type
    {
        black,
        white
    };

    Figure(Graph_lib::Window& win, Type color, std::string path_to_image)
        : Graph_lib::Image(Graph_lib::Point(0, 0), path_to_image)
    {
        this->color = color;
        win.attach(*this);
    }

    // Checks if clicked cell satisfies the conditions of a correct move
    virtual int correct_step (Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe = true) = 0;

    // Creates an object "VisualSteps" that is required to show all possible moves of currently clicked figure
    virtual VisualSteps* show_possible_steps (Coordinate position, Chessboard& chess) = 0;

    virtual bool can_take_king (Chessboard& chess, Cell& king_position) = 0;

    virtual bool is_pawn () { return false; }    /////////////////
    virtual bool is_king () { return false; }    //             //
    virtual bool is_bishop () { return false; }  //  could be   //
    virtual bool is_knight () { return false; }  // done better //
    virtual bool is_queen () { return false; }   //             //
    virtual bool is_rook () { return false; }    /////////////////

    bool has_cell () { return cell != nullptr; }

    const Cell*& get_cell () { return cell; }

    virtual bool double_step0 () { return false; }

    virtual void reset_double_step () {}

    bool change_pos_decider (Cell& c);

    void draw_lines () const override { Graph_lib::Image::draw_lines(); }

    void attach (const Cell& c);

    bool is_white () const { return color == white; }

    bool is_black () const { return color == black; }

    void detach () { cell = nullptr; }

    bool double_step{false};
    int steps_till_reset{0};
    bool first_step{true};

    private:
        bool color;
        static constexpr int r = 0.9 * (80) / 2;
        const Cell* cell{nullptr};
};

bool king_is_under_attack (Chessboard& chess, bool is_white);

struct Pawn : Figure
{
    Pawn(Graph_lib::Window& win, Figure::Type color)
        : Figure(win, color, color == Type::white ? "images/wP.png" : "images/bP.png") {}

    int correct_step (Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe = true);
    VisualSteps* show_possible_steps (Coordinate position, Chessboard& chess) override;

    bool can_take_king (Chessboard& chess, Cell& king_position) override;

    bool is_pawn () override { return true; }

    bool double_step0 () override { return double_step; }

    void reset_double_step () override;


};

struct King : Figure
{
    King(Graph_lib::Window& win, Figure::Type color) : Figure(win, color, color == Type::white ? "images/wK.png" : "images/bK.png"),
    can_do_castling{true}, possible_castlings{false, false, false, false} {};

    int correct_step (Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe = true) override;
    VisualSteps* show_possible_steps (Coordinate position, Chessboard& chess) override;
    bool is_king () override { return true; }

    bool can_take_king (Chessboard& chess, Cell& king_position) override
    {
        bool can_do_castling_reserved = can_do_castling;
        bool returning_value = (correct_step(*(const_cast<Cell*>(get_cell())), king_position, chess, false) > 0);
        can_do_castling = can_do_castling_reserved;
        return returning_value;
    }

    std::array<bool, 4> castlings () { return possible_castlings; }

    bool can_do_castling;

    private:

        std::array<bool, 4> possible_castlings; 

};

struct Bishop : Figure
{
    Bishop(Graph_lib::Window& win, Figure::Type color)
        : Figure(win, color, color == Type::white ? "images/wB.png" : "images/bB.png"){};

    int correct_step (Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe = true) override;
    VisualSteps* show_possible_steps (Coordinate position, Chessboard& chess) override;
    bool is_bishop () override { return true; }

    bool can_take_king (Chessboard& chess, Cell& king_position) override
    {
        return (correct_step(*(const_cast<Cell*>(get_cell())), king_position, chess, false) > 0);
    }
    private:
        // HF - help function
        void show_possible_steps_HF (int x, int y, int x0, int y0, int d1, int d2, VisualSteps*& steps_representation,
                                    Chessboard& chess);
};

struct Knight : Figure
{
    Knight(Graph_lib::Window& win, Figure::Type color)
        : Figure(win, color, color == Type::white ? "images/wN.png" : "images/bN.png"){};

    VisualSteps* show_possible_steps (Coordinate position, Chessboard& chess) override;

    bool is_knight () { return true; }
    int correct_step (Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe = true) override;
    bool is_king () override { return false; }
    bool can_take_king (Chessboard& chess, Cell& king_position) override
    {
        return (correct_step(*(const_cast<Cell*>(get_cell())), king_position, chess, false) > 0);
    }
};

struct Queen : Figure
{
    Queen(Graph_lib::Window& win, Figure::Type color)
        : Figure(win, color, color == Type::white ? "images/wQ.png" : "images/bQ.png"){};

    int correct_step (Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe = true) override;
    VisualSteps* show_possible_steps (Coordinate position, Chessboard& chess) override;
    virtual bool is_queen () { return true; }
    bool is_king () override { return false; }

    bool can_take_king (Chessboard& chess, Cell& king_position) override
    {
        return (correct_step(*(const_cast<Cell*>(get_cell())), king_position, chess, false) > 0);
    }

    private:
        void horisontal_possible_steps (Coordinate& position, Chessboard& chess, VisualSteps*& steps_representation);
        void vertical_possible_steps (Coordinate& position, Chessboard& chess, VisualSteps*& steps_representation);
        void diagnal_possible_steps (int x0, int y0, Coordinate& position, Chessboard& chess,
                                    VisualSteps*& steps_representation);
        void show_possible_steps_HF (int x, int y, int x0, int y0, int d1, int d2, VisualSteps*& steps_representation,
                                    Chessboard& chess);
};

struct Rook : Figure
{
    Rook(Graph_lib::Window& win, Figure::Type color) : Figure(win, color, color == Type::white ? "images/wR.png" : "images/bR.png"),
    can_do_castling{true} {};

    int correct_step (Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe = true) override;
    VisualSteps* show_possible_steps (Coordinate position, Chessboard& chess) override;
    bool is_rook () { return true; }
    bool is_king () override { return false; }

    bool can_take_king (Chessboard& chess, Cell& king_position) override
    {
        bool can_do_castling_reserved = can_do_castling;
        bool returning_value = (correct_step(*(const_cast<Cell*>(get_cell())), king_position, chess, false) > 0);
        can_do_castling = can_do_castling_reserved;
        return returning_value;
    }

    bool castling () { return can_do_castling; }

    bool can_do_castling;

    private:

        void horisontal_possible_steps (Coordinate& position, Chessboard& chess, VisualSteps*& steps_representation);
        void vertical_possible_steps (Coordinate& position, Chessboard& chess, VisualSteps*& steps_representation);
};