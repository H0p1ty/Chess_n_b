#pragma once
#include "figure.h"
#include <string>

#include <Graph_lib/GUI.h>
#include <Graph_lib/Graph.h>

constexpr int chess_green = 116;
constexpr int chess_white = 215;
constexpr int chess_light_yellow = 134;

constexpr int DFTBOF = 30; //distance_from_the_beginning_of_coordinates

struct Checker;

//stores coordinates on a chessboard from ('a', 1) to ('h', 8)
struct Coordinate
{
    Coordinate(char x_, int y_): x{x_}, y{y_} {}
    char x;
    int y;
};

using Graph_lib::Point;

struct Cell : Graph_lib::Button
{
    enum Type
    {
        black,
        white,
        dark_green
    };

    Cell(Point xy, Graph_lib::Callback cb, Type t);

    void attach (Graph_lib::Window& win) override;

    void activate ()
    {
        if (pw) pw->color(chess_light_yellow);
    }

    void deactivate () { reset_color(); }

    bool is_black () const { return type == dark_green; }

    static constexpr int size = 80;

    Point center () const { return Point{loc.x + width / 2, loc.y + height / 2}; }

    void attach_figure (Figure& ch);

    Figure& detach_figure ();

    bool has_figure () const { return figure != nullptr; }

    Figure& get_figure ();  // check for nullptr required

    Coordinate location() const; //gets pair {char x , int y} - coordinate of a cell
    private:

        Type type;
        void reset_color ();
        Figure* figure{nullptr};

};