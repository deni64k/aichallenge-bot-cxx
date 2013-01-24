#ifndef BOT_H_
#define BOT_H_

#include "State.h"

/*
    This struct represents your bot in the game of Ants
*/
struct Bot
{
    State state;

    Bot();

    void playGame();    //plays a single game of Ants

    void makeMoves();   //makes moves for a single turn
    void endTurn();     //indicates to the engine that it has made its moves

private:
    bool tryEnemyStep();

    bool tryEnemyAntStep(Spot const &);
    void performEnemyAntStep(Spot const &, Spot const &, int const, int const);

    bool tryFoodStep(Spot const &);
    void performFoodStep(Spot const &, Spot const &);

    bool tryExploreStep(Spot const &);
    void performExploreStep(Spot const &);

    bool tryEnemyHillStep(Spot const &);
    void performEnemyHillStep(Spot const &, Spot const &);

    std::vector<Spot> food, enemyAnts, enemyHills, myHills;
};

#endif //BOT_H_
