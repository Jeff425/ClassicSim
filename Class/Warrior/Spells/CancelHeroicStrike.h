#pragma once

#include "Spell.h"

class Warrior;
class WarriorSpells;

class CancelHeroicStrike : public Spell {
public:
    CancelHeroicStrike(Warrior* pchar, WarriorSpells* spells)
    ~CancelHeroicStrike() override;

private:
    Warrior* warr;
    WarriorSpells* spells;

    void spell_effect() override;
    SpellStatus is_ready_spell_specific() const override;
}