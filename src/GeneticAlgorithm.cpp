#include "GeneticAlgorithm.h"

GeneticAlgorithm::GeneticAlgorithm(struct GameConfig Config, sf::RenderWindow* window):Config(Config)
{
    //Construct individuals
    Game* individuals = new Game[Config.NumGames];
    for(int i = 0; i < Config.NumGames; i++)
    {
        Game* newGame = new Game(Config, window);
        individuals[i] = *newGame;
    }

    //initialize matrix
    BlocksShape.resize(Config.NumBlocksLine);
    BlocksAvailable.resize(Config.NumBlocksLine);
    BlocksBounds.resize(Config.NumBlocksLine);

    for(int i = 0; i < Config.NumBlocksLine; i++)
    {
        BlocksShape[i].resize(Config.NumBlocksColumn);
        BlocksAvailable[i].resize(Config.NumBlocksColumn);
        BlocksBounds[i].resize(Config.NumBlocksColumn);
    }

    //Determine pripreties of the blocks
    for(int i = 0; i < Config.NumBlocksLine; i++)
    {
        for(int j = 0; j < Config.NumBlocksColumn; j++)
        {
            sf::RectangleShape Block;
            Block.setSize(sf::Vector2f{Config.BlockWidth,Config.BlockHeight});
            Block.setFillColor(sf::Color(80*(j%4),60*((j+2)%5),127*(j%3),255));
            Block.setPosition((Config.BlockWidth + Config.BlockMargin)*i + Config.BlockMargin, Config.BlockOffset+(Config.BlockHeight + Config.BlockMargin)*j);
            BlocksShape[i][j] = Block;
            BlocksAvailable[i][j] = Config.BlockStrength;
            BlocksBounds[i][j] = Block.getGlobalBounds();
        }
    }

    stillAlive.resize(Config.NumGames);
    for(int i = 0; i < Config.NumGames; i++)
        stillAlive[i] = true;

    this->individualsAlive = individuals;
    this->window = window;
    this->NumIndividuals = Config.NumGames;

    if (!font.loadFromFile("Fonts/ComicNeue-Regular.ttf"))
    {
        std::cout<<"Loading text font error"<<std::endl;
        exit(0);
    }

    this->Generation = 1;
    this->record = 0;

    //NeuralNet title text
    textNeuralNetTitle.setFont(font);
    textNeuralNetTitle.setCharacterSize(30);
    textNeuralNetTitle.setPosition(Config.WindowWidth * 0.69, 0) ;
    textNeuralNetTitle.setString("Best player's brain: ");

    //Best score text
    textUpper.setFont(font);
    textUpper.setCharacterSize(25);
    textUpper.setPosition(Config.WindowWidth / 2, 0);

    //Generation text
    textLowerLeft.setFont(font);
    textLowerLeft.setCharacterSize(25);
    textLowerLeft.setPosition(Config.WindowWidth / 2, Config.WindowHeight / 2);

    //Generation text
    textLowerRight.setFont(font);
    textLowerRight.setCharacterSize(25);
    textLowerRight.setPosition(Config.WindowWidth * 3 / 4, Config.WindowHeight / 2);

    int offsetY = (Config.WindowHeight / 2 - (2 * Config.NodeDistance) - 2 * Config.Radius + 40) / 2;
    inputsText.resize(3);
    outputsText.resize(3);

    //Inputs text
    for (int i = 0; i < (int)inputsText.size(); i++)
    {
        inputsText[i].setFont(font);
        inputsText[i].setCharacterSize(15);
        inputsText[i].setPosition(Config.WindowWidth * 0.53, offsetY + Config.NodeDistance * i);
    }

    //Outputs text
    for (int i = 0; i < (int)outputsText.size(); i++)
    {
        outputsText[i].setFont(font);
        outputsText[i].setCharacterSize(15);
        outputsText[i].setPosition(Config.WindowWidth * 0.92, offsetY + Config.NodeDistance * i);
    }

    //Clock
    sf::Clock environmentClock;
    this->clock = environmentClock;
}

GeneticAlgorithm::~GeneticAlgorithm()
{
    delete[] individualsAlive;
}

void GeneticAlgorithm::update()
{
    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window->close();
        }

        window->clear();

        int numAlive = 0;

        for(int i = 0; i < Config.NumGames; i++)
        {
            if(stillAlive[i] == true)
            {
                numAlive++;

                //Draw individual
                individualsAlive[i].draw();

                //Update individual
                individualsAlive[i].update();

                //Check if there is a collision
                checkCollisions(i);

                //Check if game was lost
                checkGameOver(i);
            }
        }

        if (numAlive == 0)
            advanceGeneration();

        //Update time
        updateTime();

        //Draw Menu
        drawMenu();

        //Draw texts
        drawTexts();

        //Draw Blocks
        drawBlocks();

        //Draw Network
        Game* bestGame = getBestGame();
        bestGame->getNeuralNetwork()->draw();

        window->display();
    }
}

void GeneticAlgorithm::advanceGeneration()
{
    this->Generation += 1;

    this->NumIndividuals = Config.NumGames;

    //restart balls and bases
    for (int i = 0; i < (int)stillAlive.size(); i++)
    {
        stillAlive[i] = true;
        individualsAlive[i].getBreakoutsBall()->restart();
        individualsAlive[i].getBreakoutsBase()->restart();
        individualsAlive[i].setScore(0);
    }

    //restart blocks
    for(int i = 0; i < Config.NumBlocksLine; i++)
    {
        for(int j = 0; j < Config.NumBlocksColumn; j++)
        {
            BlocksShape[i][j].setFillColor(sf::Color(80*(j%4),60*((j+2)%5),127*(j%3),255));
            BlocksAvailable[i][j] = Config.BlockStrength;
        }
    }
}
void GeneticAlgorithm::checkGameOver(int index)
{
    //Check if ball fell down
    Ball* BreakoutsBall = individualsAlive[index].getBreakoutsBall();
    sf::CircleShape BallShape = BreakoutsBall->getGameBall();

    if(BallShape.getPosition().y + 2*BallShape.getRadius() > window->getSize().y)
    {
        stillAlive[index] = false;
        NumIndividuals -= 1;
    }
}

void GeneticAlgorithm::checkCollisions(int index)
{
    Ball* BreakoutsBall = individualsAlive[index].getBreakoutsBall();
    sf::FloatRect BallBounds = BreakoutsBall->getGameBall().getGlobalBounds();

    //Check collision with the blocks
    for(int i = 0; i < Config.NumBlocksLine; i++)
    {
        for(int j = 0; j < Config.NumBlocksColumn; j++)
        {
            if(BlocksAvailable[i][j] > 0 && BallBounds.intersects(BlocksBounds[i][j]) &&
               BreakoutsBall->getGameBall().getPosition().y < ((Config.BlockHeight + Config.BlockMargin) * Config.NumBlocksColumn) + Config.BlockOffset)
            {
                //Increase 1 point if collided with block
                individualsAlive[index].setScore(individualsAlive[index].getScore() + 1);

                BlocksAvailable[i][j] -= 1;
                sf::Color BlockColor = BlocksShape[i][j].getFillColor();

                //Change block opacity
                int newOpacity = (BlocksAvailable[i][j] * 255 / Config.BlockStrength);

                if (BlocksAvailable[i][j] != 0)
                    BlocksShape[i][j].setFillColor(sf::Color(BlockColor.r, BlockColor.g, BlockColor.b, newOpacity));

                //Upper collision
                float upperDistance = std::abs(BlocksBounds[i][j].top - (BallBounds.top + BallBounds.height));

                //Bottom collision
                float bottomDistance = std::abs((BlocksBounds[i][j].top + BlocksBounds[i][j].height) - BallBounds.top);

                //Right collision
                float rightDistance = std::abs((BlocksBounds[i][j].left + BlocksBounds[i][j].width) - BallBounds.left);

                //Left collision
                float leftDistance = std::abs(BlocksBounds[i][j].left - (BallBounds.left + BallBounds.width));

                //Determine which distance is the smallest to check the type of collision
                float minDistance = std::min(std::min(upperDistance, bottomDistance), std::min(rightDistance, leftDistance));

                //Vertical collision
                if(minDistance == upperDistance || minDistance == bottomDistance)
                {
                    float x = BreakoutsBall->getVel().x;
                    float y = (-1)*BreakoutsBall->getVel().y;
                    BreakoutsBall->setVel(sf::Vector2f{x, y});
                }

                //Horizontal collision
                else if(minDistance == leftDistance || minDistance == rightDistance)
                {
                    float x = (-1)*BreakoutsBall->getVel().x;
                    float y =  BreakoutsBall->getVel().y;
                    BreakoutsBall->setVel(sf::Vector2f{x, y});

                }
            }
        }
    }
}

void GeneticAlgorithm::drawMenu()
{
    //Draw lines
    sf::Vertex line1[] = {sf::Vertex(sf::Vector2f(window->getSize().x/2, 0)), sf::Vertex(sf::Vector2f(window->getSize().x/2, window->getSize().y))};
    sf::Vertex line2[] = {sf::Vertex(sf::Vector2f(window->getSize().x/2, window->getSize().y/2)), sf::Vertex(sf::Vector2f(window->getSize().x, window->getSize().y/2))};

    window->draw(line1, 2, sf::Lines);
    window->draw(line2, 2, sf::Lines);

}

void GeneticAlgorithm::drawBlocks()
{
    for(int i = 0; i < Config.NumBlocksLine; i++)
    {
        for(int j = 0; j < Config.NumBlocksColumn; j++)
        {
            if(BlocksAvailable[i][j] > 0)
                window->draw(BlocksShape[i][j]);
        }
    }
}

void GeneticAlgorithm::drawTexts()
{
    //Draw neural network title
    window->draw(textNeuralNetTitle);

    //Draw neural network inputs
    for(int i = 0; i < (int)inputsText.size(); i++)
        window->draw(inputsText[i]);

    //Draw neural network outputs
    for(int i = 0; i < (int)outputsText.size(); i++)
        window->draw(outputsText[i]);

    //Draw LowerLeft text
    std::string stringLowerLeft;
    stringLowerLeft += " Generation: " + std::to_string(Generation) + '\n';
    stringLowerLeft += " Number of individuals alive: " + std::to_string(NumIndividuals) + '\n';

    textLowerLeft.setString(stringLowerLeft);
    window->draw(textLowerLeft);

    //Draw LowerRight text
    std::string stringLowerRight;
    stringLowerRight += " All time record: " + std::to_string(record) + '\n';
    stringLowerRight += " Training time: " + updateTime() + '\n';

    textLowerRight.setString(stringLowerRight);
    window->draw(textLowerRight);

    //Draw upper text
    window->draw(textUpper);

}

std::string GeneticAlgorithm::updateTime()
{
    int time = static_cast<int>(clock.getElapsedTime().asSeconds());
    std::string timePassed = "";

    //hours
    timePassed += std::to_string(time / 3600) + "h ";

    //minutes
    timePassed += std::to_string(time / 60) + "m ";

    //seconds
    timePassed += std::to_string(time % 60) + "s";

    return timePassed;
}

Game* GeneticAlgorithm::getBestGame()
{
    int maxScore = 0;
    int indexBestGame = 0;

    for(int i = 0; i < Config.NumGames; i++)
    {
        if (maxScore < individualsAlive[i].getScore())
        {
            indexBestGame = i;
            maxScore = individualsAlive[i].getScore();
        }
    }

    //update record
    if (maxScore > record)
        record = maxScore;

    //Best player text data
    std::string stringtextUpper = " Best player: " + std::to_string(indexBestGame + 1) + '\n';
    stringtextUpper += " Best player's score: " + std::to_string(maxScore) + '\n';
    textUpper.setString(stringtextUpper);

    //Best player input text
    std::vector<double> inputLayer = individualsAlive[indexBestGame].getNeuralNetwork()->getLayers()[0].getInputs();

    inputsText[0].setString("Ball on X axis: " + std::to_string(ceilf(inputLayer[0] * 100) / 100).substr(0,4));
    inputsText[1].setString("Ball on Y axis: " + std::to_string(ceilf(inputLayer[1] * 100) / 100).substr(0,4));
    inputsText[2].setString("Base on X axis: " + std::to_string(ceilf(inputLayer[2] * 100) / 100).substr(0,4));

    //Best player output text
    std::vector<Layer> layers = individualsAlive[indexBestGame].getNeuralNetwork()->getLayers();
    std::vector<double> outputLayer = layers[layers.size() - 1].getOutputs();

    outputsText[0].setString("Left: " + std::to_string(ceilf(outputLayer[0] * 100) / 100).substr(0,4));
    outputsText[1].setString("Stationary: " + std::to_string(ceilf(outputLayer[1] * 100) / 100).substr(0,4));
    outputsText[2].setString("Right: " + std::to_string(ceilf(outputLayer[2] * 100) / 100).substr(0,4));

    return &individualsAlive[indexBestGame];
}

Game* GeneticAlgorithm::getIndividualsAlive()
{
    return this->individualsAlive;
}

std::vector<bool> GeneticAlgorithm::getStillAlive()
{
    return this->stillAlive;
}
