#pragma once
#include "cell.h"
#include <Graph_lib/Graph.h>
#include <Graph_lib/Simple_window.h>
#include <iostream>
#include <memory>

using Graph_lib::Address;
using Graph_lib::Point;
using Graph_lib::Vector_ref;
using Graph_lib::Text;

const Graph_lib::Point Chessboard_location{200,200};

// Class, that doesn't allow you to shoot yourself in the foot while
// trying to get a "Cell" value using indices
class Sub_Vector_ref
{
    public:
        Sub_Vector_ref() = default;
        Sub_Vector_ref(Graph_lib::Vector_ref<Cell> v_):  v{v_} {}

        Cell& operator[](int i);

    private:
        Graph_lib::Vector_ref<Cell> v;
};

class MyWindow : public Simple_window
{
    public:
        MyWindow(Point xy, int w, int h, const std::string& title);

    private:
        Graph_lib::Button quit_button;
        static void cb_quit (Address, Address widget);

        void quit () { hide(); }
};

Cell::Type type_of_cell (int i, int j);

struct Chessboard : MyWindow
{

    enum step_color
    {
        white,
        black,
        none
    };

    Chessboard(Point xy);

    void standard_fill();

    static constexpr int N = 8;
    static constexpr int N_max = 8;
    static_assert(N <= N_max, "do not allow board larger than N_max*N_max");

    // return a 1D array of values of a column of a 'c' coordinate
    Sub_Vector_ref operator[](char c);

    bool out_of_range (Coordinate pos);

    bool is_stalemate();

    bool is_check();

    bool is_mate();

    private:

        static constexpr int margin = 30;
        static constexpr int width = N * Cell::size + 2 * margin + 70;
        static constexpr int height = N * Cell::size + 2 * margin;
        static constexpr int standard_font_size = 40;

        step_color step_chooser;

        Vector_ref<Cell> cells;

        //these are made so they don't get out of scope
        std::vector<std::unique_ptr<Figure>> figures;

        Vector_ref<Text> texts;
        
        AttachManager<DangerSign>* am_check_sign = nullptr;

        static void cb_clicked (Address, Address widget)
        {
            auto& btn = Graph_lib::reference_to<Cell>(widget);
            dynamic_cast<Chessboard&>(btn.window()).clicked(btn);
        }

        Cell& at(char c, int i)
        {
            i--;
            int j = c - 'a';
            return cells[i*N + j];
        }

        void clicked (Cell& c);

        //decides if you can do a move on a clicked cell
        bool decide();

        void step_swap() {step_chooser = (step_chooser == step_color::white) ? step_color::black : step_color::white;}

        void reset_double_steps();

        void show_checkmate_message();
        void show_stalemate_message();

        Graph_lib::Marks x_labels;
        Graph_lib::Marks y_labels;

        Cell* selected{nullptr};

        VisualSteps* all_possible_steps;

        Cell* find_king(step_color color);
};
