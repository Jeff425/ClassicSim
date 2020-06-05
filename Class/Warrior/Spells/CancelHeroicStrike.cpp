#include "CancelHeroicStrike.h"

#include "CooldownControl.h"
#include "Warrior.h"
#include "WarriorSpells.h"

CancelHeroicStrike::CancelHeroicStrike(Warrior* pchar, WarriorSpells* spells) :
    Spell("Cancel Heroic Strike", "Assets/ability/Ability_rogue_ambush.png", pchar, new CooldownControl(pchar, 0.0), RestrictedByGcd::No, ResourceType::Rage, 0),
    warr(pchar),
    spells(spells) {}

CancelHeroicStrike::~CancelHeroicStrike() {
    delete cooldown;
}

void CancelHeroicStrike::spell_effect() {
    spells->cancel_heroic_strike();
}

void CancelHeroicStrike::is_ready_spell_specific() const {
    return spells->is_heroic_strike_queued();
}
