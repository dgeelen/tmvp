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
#include <limits>

#include <queue>
#include <algorithm>

#include "mediancut.h"

Block::Block(Point* points, int pointsLength)
{
    this->points = points;
    this->pointsLength = pointsLength;
    for(int i=0; i < NUM_DIMENSIONS; i++)
    {
        minCorner.x[i] = std::numeric_limits<unsigned char>::min();
        maxCorner.x[i] = std::numeric_limits<unsigned char>::max();
    }
}

Point * Block::getPoints()
{
    return points;
}

int Block::numPoints() const
{
    return pointsLength;
}

int Block::longestSideIndex() const
{
    int m = maxCorner.x[0] - minCorner.x[0];
    int maxIndex = 0;
    for(int i=1; i < NUM_DIMENSIONS; i++)
    {
        int diff = maxCorner.x[i] - minCorner.x[i];
        if (diff > m)
        {
            m = diff;
            maxIndex = i;
        }
    }
    return maxIndex;
}

int Block::longestSideLength() const
{
    int i = longestSideIndex();
    return maxCorner.x[i] - minCorner.x[i];
}

bool Point::operator<(const Point& p) const {
  /*sort by distance from (0, 0, 0) */
  unsigned long int d1 = (((this->x[0])*(this->x[0]))+((this->x[1])*(this->x[1]))+((this->x[2])*(this->x[2])));
  unsigned long int d2 = (((   p.x[0])*(    p.x[0])) +((    p.x[1])*(    p.x[1]))+((    p.x[2])*(    p.x[2])));
  return d1<=d2; /*/

  return ((this->x[0]<p.x[0])||(this->x[0]==p.x[0]))&&((this->x[1]<p.x[1])||(this->x[1]==p.x[1]))&&((this->x[2]<p.x[2])); /*/
  }

bool Block::operator<(const Block& rhs) const
{
    return this->longestSideLength() < rhs.longestSideLength();
}

void Block::shrink()
{
    int i,j;
    for(j=0; j<NUM_DIMENSIONS; j++)
    {
        minCorner.x[j] = maxCorner.x[j] = points[0].x[j];
    }
    for(i=1; i < pointsLength; i++)
    {
        for(j=0; j<NUM_DIMENSIONS; j++)
        {
            minCorner.x[j] = min(minCorner.x[j], points[i].x[j]);
            maxCorner.x[j] = max(maxCorner.x[j], points[i].x[j]);
        }
    }
}

std::list<Point> medianCut(Point* image, int numPoints, unsigned int desiredSize)
{
    std::priority_queue<Block> blockQueue;

    Block initialBlock(image, numPoints);
    initialBlock.shrink();

    blockQueue.push(initialBlock);
    while (blockQueue.size() < desiredSize)
    {
        Block longestBlock = blockQueue.top();
        blockQueue.pop();
        Point * begin  = longestBlock.getPoints();
  Point * median = longestBlock.getPoints() + (longestBlock.numPoints()+1)/2;
  Point * end    = longestBlock.getPoints() + longestBlock.numPoints();
  switch(longestBlock.longestSideIndex())
  {
  case 0: std::nth_element(begin, median, end, CoordinatePointComparator<0>()); break;
  case 1: std::nth_element(begin, median, end, CoordinatePointComparator<1>()); break;
  case 2: std::nth_element(begin, median, end, CoordinatePointComparator<2>()); break;
  }

  Block block1(begin, median-begin), block2(median, end-median);
  block1.shrink();
  block2.shrink();

        blockQueue.push(block1);
        blockQueue.push(block2);
    }

    std::list<Point> result;
    while(!blockQueue.empty())
    {
        Block block = blockQueue.top();
        blockQueue.pop();
        Point * points = block.getPoints();

        int sum[NUM_DIMENSIONS] = {0};
        for(int i=0; i < block.numPoints(); i++)
        {
            for(int j=0; j < NUM_DIMENSIONS; j++)
            {
                sum[j] += points[i].x[j];
            }
        }

        Point averagePoint;
        for(int j=0; j < NUM_DIMENSIONS; j++)
        {
            averagePoint.x[j] = sum[j] / block.numPoints();
        }

        result.push_back(averagePoint);
    }

    return result;
}
