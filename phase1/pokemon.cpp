#include "pokemon.h"

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
	_skillName[0] = "kick";
	_skillName[1] = "spark";
	_skillName[2] = "rage";
	_skillName[3] = "fireball";
	_skillDscp[0] = "simple attack";
	_skillDscp[1] = "ignore opponent's half defence";
	_skillDscp[2] = "increase attack";
	_skillDscp[3] = "cause huge damage";
	_pp[0] = 10;
	_pp[1] = 3;
	_pp[2] = 5;
}

bool Race_1::attack(Pokemon &attacker, Pokemon &aim, int skillIndex) const
{
	msg << attacker.name() << " uses " << attacker.skillName(skillIndex) << "!\n";
	switch (skillIndex)
	{
	case 1: //spark
	{
		int dmg = attacker.catk() + attacker.lv() * 2 - aim.cdef() / 2 + f(4);
		if (dmg < 1)
			dmg = 1;
		return aim.changeHp(-dmg);
		break;
	}
	case 2: //rage
		attacker.changeAtk(attacker.atk() / 8);
		break;
	case 3: //fireball
	{
		int dmg = attacker.catk() * 2 - aim.cdef() + 8 + f(4 + attacker.lv());
		if (dmg < 1)
			dmg = 1;
		return aim.changeHp(-dmg);
		break;
	}
	default:
	{
		//simple attack
		int dmg = attacker.catk() - aim.cdef() + f(4);
		if (dmg <= 0)
			dmg = 1;
		return aim.changeHp(-dmg);
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
	_skillDscp[2] = "cause damage and restore HP, ignore defence";
	_skillDscp[3] = "cause huge damage";
	_pp[0] = 5;
	_pp[1] = 10;
	_pp[2] = 5;
}

bool Race_2::attack(Pokemon &attacker, Pokemon &aim, int skillIndex) const
{
	msg << attacker.name() << " uses " << attacker.skillName(skillIndex) << "!\n";
	switch (skillIndex)
	{
	case 1: //photosynthesis
	{
		attacker.changeHp(attacker.catk() / 2 + attacker.cdef() + f(4));
		break;
	}
	case 2: //life drain
	{
		int dmg = attacker.catk() + f(4 + attacker.lv());
		if (dmg < 1)
			dmg = 1;
		attacker.changeHp(dmg);
		return aim.changeHp(-dmg);
		break;
	}
	case 3: //razor leaf
	{
		int dmg = attacker.catk() * 2 - aim.cdef() + f(3 + attacker.lv());
		if (dmg < 1)
			dmg = 1;
		return aim.changeHp(-dmg);
		break;
	}
	default:
	{
		//cause damage
		int dmg = attacker.catk() - aim.cdef() + f(4);
		if (dmg <= 0)
			dmg = 1;
		return aim.changeHp(-dmg);
		break;
	}
	} //switch
	return false;
}

Pokemon::Pokemon(const PokemonBase &race, const string &name) : _race(race)
{
	if (!name.length())
	{
		_name = _race.raceName();
	}
	else
	{
		_name = name;
	}

	//add some random factor
	_atk = _race.baseAtk() + f(3);
	_def = _race.baseDef() + f(2);
	_maxHp = _hp = _race.baseHp() + f(4);
	_speed = _race.baseSpeed() + f(3);

	_lv = 1;
	_exp = 0;

	for (int i = 0; i < 3; ++i)
	{
		_cpp[i] = _race.pp(i);
	}

	//output info
	msg << "Init " << _name << " from " << _race.raceName() << endl
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
		msg << "	Name: " << _race.skillName(i) << endl;
		msg << "	Description: " << _race.skillDscp(i) << endl;
		if (i)
		{
			msg << "	PP: " << _race.pp(i - 1) << endl;
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
	switch (_race.type())
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

int Pokemon::cpp(int n) const
{
	if (n >= 0 || n <= 2)
	{
		return _cpp[n];
	}
	return 0;
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
	_hp += count;

	if (_hp > _maxHp)
		_hp = _maxHp;
	if (_hp < 0)
		_hp = 0;

	if (count > 0)
	{
		msg << _name << " restores " << count << "HP!\n";
	}
	else
	{
		msg << _name << " takes " << -count << " damage!\n";
	}
	if (!_hp)
	{
		msg << _name << " is down!\n";
		return true;
	}
	else
	{
		msg << _name << "'s HP becomes " << _hp << endl;
	}
	return false;
}

void Pokemon::restoreAll()
{
	_hp = _maxHp;
	_catk = _atk;
	_cdef = _def;
	_cspeed = _speed;
	for (int i = 0; i < 3; ++i)
	{
		_cpp[i] = _race.pp(i);
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

	bool LV_UP = false;
	while (_lv < 15 && _exp > _race.expCurve(_lv - 1))
	{
		//level-up!
		LV_UP = true;
		++_lv;
		msg << "Level Up!\n";
		msg << _name << "'s now LV" << _lv << "!\n"
				<< endl;

		//increase attributes
		int atk, def, maxHp, speed;
		atk = 4 + f(1);
		def = 2 + f(1);
		maxHp = 8 + f(2);
		speed = 5 + f(1);

		//race talent
		switch (_race.type())
		{
		case ATK:
			atk += 3;
			break;
		case HP:
			maxHp += 5;
			break;
		case DEF:
			def += 3;
			break;
		case SPE:
			speed += 3;
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
	}

	if (LV_UP)
		return true;

	return false; //default
}

bool Pokemon::attack(Pokemon &aim, bool autoFight, int skillIndex)
{
	if (autoFight){
		//judge usable skill by LV and PP
		bool usable[3];
		int usableCount = 1;//can use simple attack by default
		for (int i = 0; i < 3; ++i){
			if (_lv >= (i + 1) * 5 && _cpp[i]){
				usable[i] = true;
				++usableCount;
			}
		}
		//get a random skill
		int use = rand() % usableCount;
		//find the skill
		if (!use) skillIndex = 0;
		else {
			for (int i = 0; i < 3; ++i){
				if (usable[i]){
					--use;
					if (!use){
						skillIndex = i + 1;
						break;
					}
				}
			}
		}
		if (skillIndex > 0) --_cpp[skillIndex - 1];
		return _race.attack(*this, aim, skillIndex);
	}

	//manual fight, judge skillIndex
	if (skillIndex * 5 <= _lv && _cpp[skillIndex - 1])
	{
		--_cpp[skillIndex - 1];
		return _race.attack(*this, aim, skillIndex);
	}

	return _race.attack(*this, aim, 0);
}

int f(int n)
{
	return rand() % (2 * n) - n;
}