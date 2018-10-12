#include "pokemon.h"
#include <stdlib.h>
#include <time.h>

PokemonBase::PokemonBase(PokemonType type)
{
	_type = type;

	//standard attribute
	_baseAtk = 10;
	_baseDef = 5;
	_baseHp = 25;
	_baseSpeed = 10;

	//race talent
	switch (type)
	{
	case ATK:
		_baseAtk += 3;
		break;
	case DEF:
		_baseDef += 3;
		break;
	case HP:
		_baseHp += 10;
		break;
	case SPE:
		_baseSpeed += 5;
		break;
	default:
		break;
	}
}

string PokemonBase::skillName(int n) const
{
	if (n >= 0 && n <= 3)
	{
		return _skillName[n];
	}
	return "";
}

string PokemonBase::skillDscp(int n) const
{
	if (n >= 0 && n <= 3)
	{
		return _skillDscp[n];
	}
	return "";
}

int PokemonBase::pp(int n) const
{
	if (n >= 0 && n <= 2)
	{
		return _pp[n];
	}
	return 0;
}

int PokemonBase::expCurve(int level) const
{
	if (level <= 13 && level >= 0)
	{
		return _expCurve[level];
	}
	return 0;
}

Race_1::Race_1() : PokemonBase(ATK)
{
	_raceName = "Charmander";
	_expCurve[0] = 5;
	for (int i = 1; i < 14; ++i)
	{
		_expCurve[i] = _expCurve[i - 1] + 5 * i;
	}
	_skillName[0] = "knock";
	_skillName[1] = "spark";
	_skillName[2] = "rage";
	_skillName[3] = "fireball";
	_skillDscp[0] = "simple attack";
	_skillDscp[1] = "ignore opponent's half defencce";
	_skillDscp[2] = "increase attack";
	_skillDscp[3] = "cause huge damage";
	_pp[0] = 10;
	_pp[1] = 3;
	_pp[2] = 5;
}

bool Race_1::attack(Pokemon *attacker, Pokemon *aim, int skillIndex)
{
	msg << attacker->name() << " uses " << attacker->skillName(skillIndex) << "!\n";
	switch (skillIndex)
	{
	case 1: //spark
	{
		int dmg = attacker->catk() - aim->cdef() / 2 + rand() % 4 - 2;
		if (dmg < 1)
			dmg = 1;
		return aim->changeHp(-dmg);
		break;
	}
	case 2: //rage
		attacker->changeAtk(2);
		break;
	case 3: //fireball
	{
		int dmg = attacker->catk() - aim->cdef() + 8 + rand() % 4 - 2;
		if (dmg < 1)
			dmg = 1;
		return aim->changeHp(-dmg);
		break;
	}
	default:
	{
		//simple attack
		int dmg = attacker->catk() - aim->cdef() + rand() % 4 - 2;
		if (dmg <= 0)
			dmg = 1;
		return aim->changeHp(-dmg);
		break;
	}
	} //switch
	return false;
}

Race_2::Race_2() : PokemonBase(HP)
{
	_raceName = "Bulbasaur";
	_expCurve[0] = 5;
	for (int i = 1; i < 14; ++i)
	{
		_expCurve[i] = _expCurve[i - 1] + 5 * i;
	}
	_skillName[0] = "knock";
	_skillName[1] = "photosynthesis";
	_skillName[2] = "life drain";
	_skillName[3] = "razor leaf";
	_skillDscp[0] = "simple attack";
	_skillDscp[1] = "restore HP";
	_skillDscp[2] = "cause damage and restore HP";
	_skillDscp[3] = "cause huge damage";
	_pp[0] = 5;
	_pp[1] = 10;
	_pp[2] = 5;
}

bool Race_2::attack(Pokemon *attacker, Pokemon *aim, int skillIndex)
{
	msg << attacker->name() << " uses " << attacker->skillName(skillIndex) << "!\n";
	switch (skillIndex)
	{
	case 1: //photosynthesis
	{
		attacker->changeHp(attacker->catk() / 2 + attacker->cdef() / 2 + rand() % 4 - 2);
		break;
	}
	case 2: //life drain
	{
		int dmg = attacker->catk() - aim->cdef() + rand() % 4 - 3;
		if (dmg < 1)
			dmg = 1;
		attacker->changeHp(dmg);
		return aim->changeHp(-dmg);
		break;
	}
	case 3: //razor leaf
	{
		int dmg = attacker->catk() - aim->cdef() + 8 + rand() % 4 - 2;
		if (dmg < 1)
			dmg = 1;
		return aim->changeHp(-dmg);
		break;
	}
	default:
	{
		//cause damage
		int dmg = attacker->catk() - aim->cdef() + rand() % 4 - 2;
		if (dmg <= 0)
			dmg = 1;
		return aim->changeHp(-dmg);
		break;
	}
	} //switch
	return false;
}

Pokemon::Pokemon(PokemonBase *race, const string &name)
{
	if (!race)
	{
		//TODO: error
	}

	_race = race;

	if (!name.length())
	{
		_name = _race->raceName();
	}
	else
	{
		_name = name;
	}

	//add some random factor
	_atk = _race->baseAtk() + rand() % 6 - 3;				 // +-3
	_def = _race->baseDef() + rand() % 4 - 2;				 // +-2
	_maxHp = _chp = _race->baseHp() + rand() % 8 - 4; // +-4
	_speed = _race->baseSpeed() + rand() % 4 - 2;		 // +-2

	_lv = 1;
	_exp = 0;

	for (int i = 0; i < 4; ++i)
	{
		_skillPriority[i] = 0;
	}
	for (int i = 0; i < 3; ++i)
	{
		_pp[i] = _race->pp(i);
	}

	//output info
	msg << "Init " << _name << " from " << _race->raceName() << endl
			<< "Type: " << raceType() << endl
			<< "Atk: " << _atk << endl
			<< "Def: " << _def << endl
			<< "MaxHp: " << _maxHp << endl
			<< "Speed: " << _speed << endl
			<< "LV: " << _lv << endl
			<< "Exp: " << _exp << endl;
	//output skill
	msg << "Skills:\n";
	for (int i = 0; i < 4; ++i)
	{
		msg << "	Name: " << _race->skillName(i) << endl;
		msg << "	Description: " << _race->skillDscp(i) << endl;
		if (i)
		{
			msg << "	PP: " << _race->pp(i - 1) << endl;
		}
		else
		{
			msg << "	PP: infinity\n";
		}
	}
	msg << endl;
}

string Pokemon::raceType() const
{
	switch (_race->type())
	{
	case ATK:
		return "High Attack";
	case HP:
		return "High HP";
	case DEF:
		return "High Defence";
	case SPE:
		return "High Speed";
	default:
		break;
	}
	return "";
}

void Pokemon::changeAtk(int count)
{
	_catk += count;
	if (_catk < 1)
		_catk = 1;

	if (count > 0)
	{
		msg << _name << "'s Attack +" << count << endl;
	}
	else
	{
		msg << _name << "'s Attack " << count << endl;
	}

	msg << _name << "'s Attack becomes " << _catk << endl
			<< endl;
}

void Pokemon::changeDef(int count)
{
	_cdef += count;
	if (_cdef < 1)
		_cdef = 1;

	if (count > 0)
	{
		msg << _name << "'s Defence +" << count << endl;
	}
	else
	{
		msg << _name << "'s Defence " << count << endl;
	}
	msg << _name << "'s Defence becomes " << _cdef << endl
			<< endl;
}

void Pokemon::changeSpeed(int count)
{
	_cspeed += count;
	if (_cspeed < 1)
		_cspeed = 1;

	if (count > 0)
	{
		msg << _name << "'s Speed +" << count << endl;
	}
	else
	{
		msg << _name << "'s Speed " << count << endl;
	}
	msg << _name << "'s Speed becomes " << _cspeed << endl
			<< endl;
}

bool Pokemon::changeHp(int count)
{
	_chp += count;

	if (_chp > _maxHp)
		_chp = _maxHp;
	if (_chp < 0)
		_chp = 0;

	if (count > 0)
	{
		msg << _name << " restores " << count << "HP!\n";
	}
	else
	{
		msg << _name << " takes " << -count << " damage!\n";
	}
	if (!_chp)
	{
		msg << _name << " is down!\n";
		return true;
	}
	else
	{
		msg << _name << "'s HP becomes " << _chp << endl;
	}
	return false;
}

void Pokemon::restoreAll()
{
	_chp = _maxHp;
	_catk = _atk;
	_cdef = _def;
	_cspeed = _speed;
	for (int i = 0; i < 3; ++i)
	{
		_cpp[i] = _race->pp(i);
	}
}

bool Pokemon::getExp(int count)
{
	if (_lv == 15)
		return false;

	_exp += count;

	msg << _name << " gains " << count << " exp!\n";
	msg << "Now " << _name << " has " << _exp << " exp\n"
			<< endl;

	if (_exp > _race->expCurve(_lv))
	{
		//level-up!
		++_lv;
		msg << "Level Up!\n";
		msg << _name << "'s now LV" << _lv << "!\n"
				<< endl;

		//increase attributes
		int atk, def, maxHp, speed;
		atk = 4 + rand() % 4 - 2;		// +-2
		def = 2 + rand() % 2 - 1;		//+-1
		maxHp = 5 + rand() % 4 - 2; //+-2
		speed = 2 + rand() % 2 - 1; //+-1

		//race talent
		switch (_race->type())
		{
		case ATK:
			atk += 2;
			break;
		case HP:
			maxHp += 4;
			break;
		case DEF:
			def += 2;
			break;
		case SPE:
			speed += 1;
			break;
		default:
			break;
		}

		_atk += atk;
		_def += def;
		_maxHp += maxHp;
		_speed += speed;

		msg << "Atk: " << _atk - atk << "->" << _atk << "!\n";
		msg << "Def: " << _def - def << "->" << _def << "!\n";
		msg << "MaxHP: " << _maxHp - maxHp << "->" << _maxHp << "!\n";
		msg << "Speed: " << _speed - speed << "->" << _speed << "!\n\n";

		return true;
	}

	return false; //default
}

bool Pokemon::attack(Pokemon *aim, int skillIndex){
	if (skillIndex >= 1 && skillIndex <= 3){
		if (_cpp[skillIndex - 1] > 0){
			--_cpp[skillIndex - 1];
			return _race->attack(this, aim, skillIndex);
		}
	}

	return _race->attack(this, aim, 0);
}