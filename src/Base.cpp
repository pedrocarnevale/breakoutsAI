#include "Base.h"

Base::Base(int BaseVel, float BaseWidth, float BaseHeight, enum Mode GameType,  sf::Color BaseColor,  sf::RenderWindow* window):BaseVel(BaseVel),BaseWidth(BaseWidth),BaseHeight(BaseHeight),BaseColor(BaseColor),window(window)
{
    BaseShape.setSize(sf::Vector2f{BaseWidth,BaseHeight});
    BaseShape.setFillColor(BaseColor);

    int WindowSize = window->getSize().x;
    if(GameType == Mode::NEURAL_NETWORK)
        WindowSize /= 2;
    BaseShape.setPosition((WindowSize/2 - BaseWidth/2),window->getSize().y - 50);

    this->BaseDirection = Direction::STATIONARY;
}

void Base::update(double Left, double Right, enum Mode GameType)
{
    if(GameType == Mode::NEURAL_NETWORK)
    {
        if(Left > Right)
        {
            Left = 1;
            Right = 0;
        }
        else
        {
            Left = 0;
            Right = 1;
        }

    }

    int BasePosition = BaseShape.getPosition().x;

    int WindowSize = static_cast<int>(window->getSize().x);
    if(GameType == Mode::NEURAL_NETWORK)
        WindowSize /= 2;

    //Move right
    if(Right && (BasePosition + BaseWidth < WindowSize))
    {
        BaseShape.move(BaseVel,0);
        BaseDirection = Direction::RIGHT;
    }

    //Move left
    else if(Left && BasePosition > 0)
    {
        BaseShape.move(-BaseVel,0);
        BaseDirection = Direction::LEFT;
    }

    else
        BaseDirection = Direction::STATIONARY;

    //Check if is out of screen
    if(BasePosition + BaseWidth > WindowSize)
        BaseShape.setPosition(WindowSize - BaseWidth, window->getSize().y - 50);
}

void Base::draw()
{
    window->draw(BaseShape);
}

void Base::restart(enum Mode GameType)
{
    int WindowSize = static_cast<int>(window->getSize().x);
    if(GameType == Mode::NEURAL_NETWORK)
        WindowSize /= 2;

    BaseShape.setPosition(WindowSize/2 - BaseWidth, window->getSize().y - 50);
    this->BaseDirection = Direction::STATIONARY;
}

int Base::getBaseVel() const
{
    return BaseVel;
}

int Base::getBaseWidth() const
{
    return BaseVel;
}

int Base::getBaseHeight() const
{
    return BaseVel;
}

enum Direction Base::getDirection()
{
    return BaseDirection;
}

sf::RectangleShape& Base::getBaseShape()
{
    return BaseShape;
}
