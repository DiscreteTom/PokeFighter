#include "pokemon.h"

const PokemonBase *Pokemon::races[4] = {new Race<0>(), new Race<1>(), new Race<2>(), new Race<3>()};

Pokemon *Pokemon::getEnemy(int raceIndex, int lv)
{
	if (raceIndex < 0 || raceIndex > 3)
		return 0;
	if (lv < 1 || lv > 15)
		return 0;
	Pokemon *result = new Pokemon(raceIndex);
	while (result->lv() < lv)
	{
		result->gainExp(10);
	}
	return result;
}

PokemonBase::PokemonBase(PokemonType type)
{
	_type = type;

	//standard attribute
	_baseAtk = 10;
	_baseDef = 5;
	_baseHp = 25;
	_baseSpeed = 10;

	//type talent
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
	if (level <= 15 && level >= 2)
	{
		return _expCurve[level - 2];
	}
	return 0;
}

bool PokemonBase::dodge(int attacker, int aim, string &msg) const
{
	if ((attacker + f(40)) / 80 - aim / 150 < 0)
	{
		// dbout << "Miss!\n";
		msg += "1 ";
		return true;
	}
	msg += "0 ";
	return false;
}

Pokemon::Pokemon(int raceIndex, const string &name)
{
	_raceIndex = raceIndex;

	if (!name.length())
	{
		_name = races[_raceIndex]->raceName(); //use race name as default name
	}
	else
	{
		_name = name;
	}

	//add some random factor
	_atk = races[_raceIndex]->baseAtk() + f(3);
	_def = races[_raceIndex]->baseDef() + f(2);
	_maxHp = _hp = races[_raceIndex]->baseHp() + f(5);
	_speed = races[_raceIndex]->baseSpeed() + f(3);

	_lv = 1;
	_exp = 0;

	for (int i = 0; i < 3; ++i)
	{
		_cpp[i] = races[_raceIndex]->pp(i);
	}

	//output info
	// dbout << "Init " << _name << " from " << races[_raceIndex]->raceName() << endl
	// << "Type: " << raceType() << endl
	// << "Atk: " << _atk << endl
	// << "Def: " << _def << endl
	// << "MaxHp: " << _maxHp << endl
	// << "Speed: " << _speed << endl
	// << "LV: " << _lv << endl
	// << "Exp: " << _exp << endl;
	//output skill
	// dbout << "Skills:\n";
	for (int i = 0; i < 4; ++i)
	{
		// dbout << "	Name: " << races[_raceIndex]->skillName(i) << endl;
		// dbout << "	Description: " << races[_raceIndex]->skillDscp(i) << endl;
		if (i)
		{
			// dbout << "	PP: " << races[_raceIndex]->pp(i - 1) << endl;
		}
		else
		{
			// dbout << "	PP: infinity\n";
		}
	}
	// dbout << endl;
}

Pokemon::Pokemon(const string &name, int raceIndex, int atk, int def, int maxHp, int speed, int lv, int exp)
{
	_name = name;
	_raceIndex = raceIndex;
	_atk = atk;
	_def = def;
	_maxHp = maxHp;
	_speed = speed;
	_lv = lv;
	_exp = exp;
}

Pokemon::~Pokemon()
{
}

string Pokemon::raceType() const
{
	switch (races[_raceIndex]->type())
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
		// dbout << _name << "'s Attack +" << count << endl;
	}
	else
	{
		// dbout << _name << "'s Attack " << count << endl;
	}

	// dbout << _name << "'s Attack becomes " << _catk << endl
	// << endl;
}

void Pokemon::changeDef(int count)
{
	_cdef += count;
	if (_cdef < 1)
		_cdef = 1;

	if (count > 0)
	{
		// dbout << _name << "'s Defence +" << count << endl;
	}
	else
	{
		// dbout << _name << "'s Defence " << count << endl;
	}
	// dbout << _name << "'s Defence becomes " << _cdef << endl
	// << endl;
}

void Pokemon::changeSpeed(int count)
{
	_cspeed += count;
	if (_cspeed < 1)
		_cspeed = 1;

	if (count > 0)
	{
		// dbout << _name << "'s Speed +" << count << endl;
	}
	else
	{
		// dbout << _name << "'s Speed " << count << endl;
	}
	// dbout << _name << "'s Speed becomes " << _cspeed << endl
	// << endl;
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
		// dbout << _name << " restores " << count << "HP!\n";
	}
	else
	{
		// dbout << _name << " takes " << -count << " damage!\n";
	}
	if (!_hp)
	{
		// dbout << _name << " is down!\n\n";
		return true;
	}
	else
	{
		// dbout << _name << "'s HP becomes " << _hp << endl
		// << endl;
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
		_cpp[i] = races[_raceIndex]->pp(i);
	}
}

bool Pokemon::gainExp(int count)
{
	if (_lv == 15)
		return false;

	if (count <= 0)
		count = 1;

	_exp += count;

	// dbout << _name << " gains " << count << " exp!\n";
	// dbout << "Now " << _name << " has " << _exp << " exp\n"
	// << endl;

	bool LV_UP = false;
	while (_lv < 15 && _exp > races[_raceIndex]->expCurve(_lv + 1))
	{
		//level-up!
		LV_UP = true;
		++_lv;
		// dbout << "Level Up!\n";
		// dbout << _name << "'s now LV" << _lv << "!\n"
		// << endl;

		//increase attributes
		int atk, def, maxHp, speed;
		atk = 4 + f(1);
		def = 2 + f(1);
		maxHp = 8 + f(2);
		speed = 5 + f(1);

		//race talent
		switch (races[_raceIndex]->type())
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

		// dbout << "Atk: " << _atk - atk << "->" << _atk << "!\n";
		// dbout << "Def: " << _def - def << "->" << _def << "!\n";
		// dbout << "MaxHP: " << _maxHp - maxHp << "->" << _maxHp << "!\n";
		// dbout << "Speed: " << _speed - speed << "->" << _speed << "!\n\n";
	}

	if (LV_UP)
		return true;

	return false; //default
}

// auto attack
bool Pokemon::attack(Pokemon &aim, string &msg)
{
	int skillIndex = 0;
	//judge usable skill by LV and PP
	bool usable[3];
	int usableCount = 1;				//can use 普通攻击 by default
	for (int i = 0; i < 3; ++i) //get all usable skill
	{
		if (_lv >= (i + 1) * 5 && _cpp[i])
		{
			usable[i] = true;
			++usableCount;
		}
	}
	//get a random skill
	int use = rand() % usableCount;
	//find the skill
	if (!use)
		skillIndex = 0;
	else
	{
		for (int i = 0; i < 3; ++i)
		{
			if (usable[i])
			{
				--use;
				if (!use)
				{
					skillIndex = i + 1;
					break;
				}
			}
		}
	}
	if (skillIndex > 0)
		--_cpp[skillIndex - 1]; //consume pp
	return races[_raceIndex]->attack(*this, aim, msg, skillIndex);
}

// manual attack
bool Pokemon::attack(Pokemon &aim, int skillIndex, string &msg)
{
	//manual fight, get skillIndex
	// dbout << _name << ", your turn!\n";
	// dbout << "Choose a skill to attack!\n";
	int space = 0; //count space
	for (int i = 0; i * 5 <= _lv; ++i)
	{
		if (skillName(i).length() > space)
			space = skillName(i).length();
	}
	space += 4;
	for (int i = 0; i * 5 <= _lv; ++i)
	{
		// dbout << "	" << i + 1 << ": " << skillName(i);
		//print PP
		if (i)
		{
			// dbout << '(';
			if (_cpp[i - 1] < 10)
			{
				// dbout << ' ';
			}
			// dbout << _cpp[i - 1] << '/';
			if (races[_raceIndex]->pp(i - 1) < 10)
			{
				// dbout << ' ';
			}
			// dbout << races[_raceIndex]->pp(i - 1) << ')';
		}
		for (int j = 0; j < space - skillName(i).length(); ++j)
			// dbout << " ";
			if (!i)
			{
				// dbout << "       ";
			}
		// dbout << skillDscp(i) << endl;
	}
	// dbout << "Please input a number: ";
	// dbin >> skillIndex;
	// dbin.clear();
	// dbin.sync();
	// dbin.ignore();
	// dbout << endl;

	//attack
	if (skillIndex < 0 || skillIndex > 3)
		skillIndex = 0;
	if (skillIndex * 5 <= _lv && _cpp[skillIndex - 1]) //check again by LV and PP
	{
		--_cpp[skillIndex - 1]; //consume PP
		return races[_raceIndex]->attack(*this, aim, msg, skillIndex);
	}

	return races[_raceIndex]->attack(*this, aim, msg, 0);
}

bool Pokemon::takeDamage(int n)
{
	if (n < 10)
		n = 10;
	return changeHp(-n);
}

int f(int n)
{
	return rand() % (2 * n + 1) - n;
}

template <>
Race<0>::Race() : PokemonBase(ATK)
{
	// _raceName = "Charmander";
	_raceName = "小火龙";
	_expCurve[0] = 5;
	for (int i = 1; i < 14; ++i)
	{
		_expCurve[i] = _expCurve[i - 1] + 5 * i;
	}
	_skillName[0] = "撞击";
	_skillName[1] = "火花";
	_skillName[2] = "怒气";
	_skillName[3] = "火球";
	_skillDscp[0] = "普通攻击";
	_skillDscp[1] = "忽略敌人一半防御的攻击";
	_skillDscp[2] = "增加攻击力";
	_skillDscp[3] = "伤害很高的大招";
	_pp[0] = 10;
	_pp[1] = 3;
	_pp[2] = 5;
}

template <>
bool Race<0>::attack(Pokemon &attacker, Pokemon &aim, string &msg, int skillIndex) const
{
	// dbout << attacker.name() << " uses " << attacker.skillName(skillIndex) << "!\n";

	msg += attacker.skillName(skillIndex) + ' ';

	switch (skillIndex)
	{
	case 1: //spark
	{
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() + attacker.lv() * 2 - aim.cdef() / 2 + f(4);
		// return aim.takeDamage(dmg);
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;

		break;
	}
	case 2:				 //rage
		msg += "0 "; // can not dodge

		attacker.changeAtk(attacker.atk() / 8);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 2 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		break;
	case 3: //fireball
	{
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() * 1.5 - aim.cdef() + 8 + f(4 + attacker.lv());
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;

		break;
	}
	default:
	{
		//普通攻击
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() - aim.cdef() + f(4);
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;

		break;
	}
	} //switch
	return false;
}

template <>
Race<1>::Race() : PokemonBase(HP)
{
	// _raceName = "Bulbasaur";
	_raceName = "妙蛙种子";
	_expCurve[0] = 5;
	for (int i = 1; i < 14; ++i)
	{
		_expCurve[i] = _expCurve[i - 1] + 5 * i;
	}
	_skillName[0] = "撞击";
	_skillName[1] = "光合作用";
	_skillName[2] = "吸血";
	_skillName[3] = "飞叶快刀";
	_skillDscp[0] = "普通攻击";
	_skillDscp[1] = "恢复生命值";
	_skillDscp[2] = "无视防御，造成伤害并恢复自身生命值";
	_skillDscp[3] = "伤害很高的大招";
	_pp[0] = 5;
	_pp[1] = 10;
	_pp[2] = 5;
}

template <>
bool Race<1>::attack(Pokemon &attacker, Pokemon &aim, string &msg, int skillIndex) const
{
	// dbout << attacker.name() << " uses " << attacker.skillName(skillIndex) << "!\n";

	msg += attacker.skillName(skillIndex) + ' ';

	switch (skillIndex)
	{
	case 1: //photosynthesis
	{
		msg += "0 "; // can not dodge

		attacker.changeHp(attacker.catk() / 2 + attacker.cdef() + f(4));
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		break;
	}
	case 2: //life drain
	{
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() + f(4 + attacker.lv());
		if (dmg < 10)
			dmg = 10;
		attacker.changeHp(dmg);
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;
		break;
	}
	case 3: //razor leaf
	{
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() * 2 - aim.cdef() + f(3 + attacker.lv());
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;

		break;
	}
	default:
	{
		//普通攻击
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() - aim.cdef() + f(4);
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;

		break;
	}
	} //switch
	return false;
}

template <>
Race<2>::Race() : PokemonBase(DEF)
{
	// _raceName = "Squirtle";
	_raceName = "杰尼龟";
	_expCurve[0] = 5;
	for (int i = 1; i < 14; ++i)
	{
		_expCurve[i] = _expCurve[i - 1] + 5 * i;
	}
	_skillName[0] = "撞击";
	_skillName[1] = "铁壁";
	_skillName[2] = "水之波动";
	_skillName[3] = "水炮";
	_skillDscp[0] = "普通攻击";
	_skillDscp[1] = "增加防御";
	_skillDscp[2] = "造成伤害并增加攻击";
	_skillDscp[3] = "伤害很高的大招";
	_pp[0] = 10;
	_pp[1] = 10;
	_pp[2] = 3;
}

template <>
bool Race<2>::attack(Pokemon &attacker, Pokemon &aim, string &msg, int skillIndex) const
{
	// dbout << attacker.name() << " uses " << attacker.skillName(skillIndex) << "!\n";

	msg += attacker.skillName(skillIndex) + ' ';

	switch (skillIndex)
	{
	case 1: //iron defence
	{
		msg += "0 "; // can not dodge

		attacker.changeDef(2);
		// bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 2 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		// return result;
		break;
	}
	case 2: //water pulse
	{
		attacker.changeAtk(2);
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() - aim.cdef() + f(4 + attacker.lv());
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 2 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;

		break;
	}
	case 3: //hydro pump
	{
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() * 2 - aim.cdef() + f(3 + attacker.lv());
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;

		break;
	}
	default:
	{
		//普通攻击
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() - aim.cdef() + f(4);
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;

		break;
	}
	} //switch
	return false;
}

template <>
Race<3>::Race() : PokemonBase(SPE)
{
	// _raceName = "Pidgey";
	_raceName = "波波";
	_expCurve[0] = 5;
	for (int i = 1; i < 14; ++i)
	{
		_expCurve[i] = _expCurve[i - 1] + 5 * i;
	}
	_skillName[0] = "撞击";
	_skillName[1] = "高速移动";
	_skillName[2] = "拍打";
	_skillName[3] = "猛冲";
	_skillDscp[0] = "普通攻击";
	_skillDscp[1] = "提升自身速度";
	_skillDscp[2] = "自身速度越高伤害越高";
	_skillDscp[3] = "伤害很高的大招。自身速度越高伤害越高";
	_pp[0] = 5;
	_pp[1] = 10;
	_pp[2] = 5;
}

template <>
bool Race<3>::attack(Pokemon &attacker, Pokemon &aim, string &msg, int skillIndex) const
{
	// dbout << attacker.name() << " uses " << attacker.skillName(skillIndex) << "!\n";

	msg += attacker.skillName(skillIndex) + ' ';

	switch (skillIndex)
	{
	case 1: //agility
	{
		msg += "0 "; // can not dodge

		attacker.changeSpeed(attacker.speed() / 5);
		// bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 2 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		// return result;
		break;
	}
	case 2: //wing attack
	{
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() + attacker.cspeed() / 4 - aim.cdef() + f(4 + attacker.lv());
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;

		break;
	}
	case 3: //take down
	{
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() - aim.cdef() + attacker.cspeed() / 2 + f(3 + attacker.lv());
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;

		break;
	}
	default:
	{
		//普通攻击
		if (dodge(attacker.cspeed(), aim.cspeed(), msg))
			return false;

		int dmg = attacker.catk() - aim.cdef() + f(4);
		bool result = aim.takeDamage(dmg);
		msg += to_string(aim.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(aim.cpp(i)) + ' ';
		}
		msg += to_string(attacker.hp()) + " 1 1 1 ";
		for (int i = 0; i < 3; ++i)
		{
			msg += to_string(attacker.cpp(i)) + ' ';
		}
		return result;

		break;
	}
	} //switch
	return false;
}