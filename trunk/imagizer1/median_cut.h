/* Copyright (c) 2006 the authors listed at the following URL:
http://literateprograms.org/Median_cut_algorithm_(C_Plus_Plus)?action=history&offset=20060531211136

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from: http://literateprograms.org/Median_cut_algorithm_(C_Plus_Plus)?oldid=5491
*/

#ifndef _MEDIAN_CUT_H_
#include <list>

const int NUM_DIMENSIONS = 3;

struct APoint
{
    unsigned char x[NUM_DIMENSIONS];
};

class Block
{
    APoint minCorner, maxCorner;
    APoint* APoints;
    int APointsLength;
public:
    Block(APoint* APoints, int APointsLength);
    APoint * getAPoints();
    int numAPoints() const;
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
class CoordinateAPointComparator
{
public:
    bool operator()(APoint left, APoint right)
    {
        return left.x[index] < right.x[index];
    }
};

std::list<APoint> medianCut(APoint* image, int numAPoints, unsigned int desiredSize);
#endif /* #ifndef _MEDIAN_CUT_H_ */

