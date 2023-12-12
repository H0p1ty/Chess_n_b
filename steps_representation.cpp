#include "steps_representation.h"
#include "board.h"



void Detacher::detach(Shape& fig) { chess->detach(fig); }


DangerSign::DangerSign(Point center, Chessboard& chess_) : Circle{center, c_size/2}, det{chess_}
{
    for(int i = center.x - c_size/4; i <= center.x + c_size/4; i += c_size/2)
    {
        for(int j = center.y - c_size/4; j <= center.y + c_size/4; j += c_size/2)
        {
            Circle* temporary_circle = new Circle{{i,j}, circle_radius};
            circle_of_circles.push_back(temporary_circle);
            circle_of_circles[circle_of_circles.size()-1].set_fill_color(Graph_lib::Color::red);
            circle_of_circles[circle_of_circles.size()-1].set_color(Graph_lib::Color::red);
        }
    }
    for(int i = center.x - c_size/4 - c_size/8; i <= center.x + c_size/4 + c_size/8; i += c_size/4 + c_size/8)
    {
        Circle* temporary_circle = new Circle{{i,center.y}, circle_radius};
        circle_of_circles.push_back(temporary_circle);
        circle_of_circles[circle_of_circles.size()-1].set_fill_color(Graph_lib::Color::red);
        circle_of_circles[circle_of_circles.size()-1].set_color(Graph_lib::Color::red);
    }
    for(int j = center.y - c_size/4 - c_size/8; j <= center.y + c_size/4 + c_size/8; j += c_size/2 + c_size/4)
    {
        Circle* temporary_circle = new Circle{{center.x,j}, circle_radius};
        circle_of_circles.push_back(temporary_circle);
        circle_of_circles[circle_of_circles.size()-1].set_fill_color(Graph_lib::Color::red);
        circle_of_circles[circle_of_circles.size()-1].set_color(Graph_lib::Color::red);
    }
}

//I am very unsure of this destructor
DangerSign::~DangerSign()
{
    for(int i = (int)circle_of_circles.size() - 1; i >= 0; i--)
    {
        //Graph_lib::Circle::~Circle();
        det.detach(circle_of_circles[i]);
    }
    det.detach(*this);
}

void DangerSign::draw_lines() const
{
    for(int i = 0; i < int(circle_of_circles.size()); i++)
        circle_of_circles[i].draw_lines();
}

RedCross::RedCross(Point center, Chessboard& chess_) : Rectangle{center, c_size, c_size}, det{chess_}
{
    int x = center.x;
    int y = center.y;

    Closed_polyline* rectangle_1 = new Closed_polyline{{x - c_size/2 + 2*dist, y - c_size/2 + dist},{x + c_size/2 - dist, y + c_size/2 - 2*dist},
                                      {x + c_size/2 - 2*dist, y + c_size/2 - dist}, {x - c_size/2 + dist, y - c_size/2 + 2*dist}};
    rectangle_1->set_color(Graph_lib::Color::red);
    rectangle_1->set_fill_color(Graph_lib::Color::red);

    Closed_polyline* rectangle_2 = new Closed_polyline{{x + c_size/2 - 2*dist, y - c_size/2 + dist},{x + c_size/2 - dist, y - c_size/2 + 2*dist},
                                      {x - c_size/2 + 2*dist, y + c_size/2 - dist},{x - c_size/2 + dist, y + c_size/2 - 2*dist}};
    rectangle_2->set_color(Graph_lib::Color::red);
    rectangle_2->set_fill_color(Graph_lib::Color::red);

    rectangles.push_back(rectangle_1);
    rectangles.push_back(rectangle_2);
}

RedCross::~RedCross()
{
    //Graph_lib::Rectangle::~Rectangle();
    det.detach(rectangles[0]);
    det.detach(rectangles[1]);
    det.detach(*this);
}

void RedCross::draw_lines() const
{
    rectangles[0].draw_lines();
    rectangles[1].draw_lines();
}

Frame::Frame(Point center, Chessboard& chess_) : 
    Rectangle({center.x - c_size/2, center.y - c_size/2}, c_size, c_size), det{chess_}
{

    Rectangle* h_r1 = new Rectangle{{center.x - c_size/2, center.y - c_size/2}, rc_width, rc_length};
    Rectangle* h_r2 = new Rectangle{{center.x + c_size/2 - rc_width, center.y - c_size/2}, rc_width, rc_length};
    Rectangle* h_r3 = new Rectangle{{center.x - c_size/2, center.y + c_size/2 - rc_length}, rc_width, rc_length};
    Rectangle* h_r4 = new Rectangle{{center.x + c_size/2 - rc_width, center.y + c_size/2 - rc_length}, rc_width, rc_length};

    horisontal_rectangles.push_back(h_r1);
    horisontal_rectangles.push_back(h_r2);
    horisontal_rectangles.push_back(h_r3);
    horisontal_rectangles.push_back(h_r4);

    //There are probably ways to avoid Copy+Paste but there's 
    //something with Graph_lib that doesn't allow it to be easy

    Rectangle* v_r1 = new Rectangle{{center.x - c_size/2, center.y - c_size/2}, rc_length, rc_width};
    Rectangle* v_r2 = new Rectangle{{center.x + c_size/2 - rc_length, center.y - c_size/2}, rc_length, rc_width};
    Rectangle* v_r3 = new Rectangle{{center.x - c_size/2, center.y + c_size/2 - rc_width}, rc_length, rc_width};
    Rectangle* v_r4 = new Rectangle{{center.x + c_size/2 - rc_length, center.y + c_size/2 - rc_width}, rc_length, rc_width};

    vertical_rectangles.push_back(v_r1);
    vertical_rectangles.push_back(v_r2);
    vertical_rectangles.push_back(v_r3);
    vertical_rectangles.push_back(v_r4);

    for(int i = 0; i < 4; i++)
    {
        horisontal_rectangles[i].set_color(chess_yellow);
        horisontal_rectangles[i].set_fill_color(chess_yellow);

        vertical_rectangles[i].set_color(chess_yellow);
        vertical_rectangles[i].set_fill_color(chess_yellow);
    }
}

//I am very unsure of this destructor
Frame::~Frame()
{
    for(int i = (int)horisontal_rectangles.size() - 1; i >= 0; i--)
    {
        //Graph_lib::Rectangle::~Rectangle();
        det.detach(horisontal_rectangles[i]);
        det.detach(vertical_rectangles[i]);
    }
    det.detach(*this);
}

void Frame::draw_lines() const
{
    for(int i = 0; i < 4; i++)
    {
        horisontal_rectangles[i].draw_lines();
        vertical_rectangles[i].draw_lines();
    }
}

//I am very unsure of this destructor
VisualSteps::~VisualSteps()
{
    for(int i = int(possible_steps.size() - 1); i >= 0; i--)
        det.detach(possible_steps[i]);
}