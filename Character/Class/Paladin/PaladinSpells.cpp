
#include "PaladinSpells.h"
#include "Paladin.h"

#include "MainhandAttack.h"


PaladinSpells::PaladinSpells(Paladin* paladin) :
    Spells(paladin),
    paladin(paladin)
{
    this->mh_attack = new MainhandAttack(paladin);

    spells.append(mh_attack);
}

PaladinSpells::~PaladinSpells() = default;
