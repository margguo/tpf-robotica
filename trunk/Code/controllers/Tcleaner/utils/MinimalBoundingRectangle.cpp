#include <utils/MinimalBoundingRectangle.h>

namespace utils{

MinimalBoundingRectangle::MinimalBoundingRectangle(int x,int y,int height,int width)
{
  this->height=height;
  this->width=width;
  this->x=x;
  this->y=y;
}

int MinimalBoundingRectangle::getTopX()
{
    return x;
}

int MinimalBoundingRectangle::getTopY()
{
    return y;
}

int MinimalBoundingRectangle::getWidth()
{
    return width;
}

int MinimalBoundingRectangle::getHeight()
{
    return height;
}

}
