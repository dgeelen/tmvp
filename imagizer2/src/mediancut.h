/***************************************************************************
 *   Copyright (C) 2006 by Da Fox   *
 *   dafox@shogoki   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef MEDIAN_CUT_H
#define MEDIAN_CUT_H
#include <list>

const int NUM_DIMENSIONS = 3;

/*struct Point {
    unsigned char x[NUM_DIMENSIONS];
    unsigned long int d;
}; */

class Point {
  public:
  unsigned char x[NUM_DIMENSIONS];
  bool operator<(const Point& p) const;
  private:
  };

class Block
{
    Point minCorner, maxCorner;
    Point* points;
    int pointsLength;
public:
    Block(Point* points, int pointsLength);
    Point * getPoints();
    int numPoints() const;
    int longestSideIndex() const;
    int longestSideLength() const;
    bool operator<(const Block& rhs) const;
    void shrink();
private:
    template <typename T>
    static T min(const T a, const T b)
    {
        if (a < b)
            return a;
        else
            return b;
    }

    template <typename T>
    static T max(const T a, const T b)
    {
        if (a > b)
            return a;
        else
            return b;
    }

};

template <int index>
class CoordinatePointComparator
{
public:
    bool operator()(Point left, Point right)
    {
        return left.x[index] < right.x[index];
    }
};

std::list<Point> medianCut(Point* image, int numPoints, unsigned int desiredSize);


#endif
