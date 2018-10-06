#pragma once

#include <string>

using std::string;

class Pokemon; //declaration

enum PokemonType
{
	ATK, //high attack
	HP,	//high HP
	DEF, //high defence
	SPE	//high speed
};

class PokemonBase
{
protected:
	string _raceName;
	PokemonType _type;
	int _baseAtk;
	int _baseDef;
	int _baseHp;
	int _baseSpeed;
	int _expCurve[14];
	string _skillName[4];
	string _skillDscp[4]; //skill description
	int _pp[3];				 //power point, first skill has infinite pp

public:
	PokemonBase(PokemonType type);

	//getter
	string raceName() const { return _raceName; }
	PokemonType type() const { return _type; }
	int baseAtk() const { return _baseAtk; }
	int baseDef() const { return _baseDef; }
	int baseHp() const { return _baseHp; }
	int baseSpeed() const { return _baseSpeed; }
	string skillName(int n) const;
	string skillDscp(int n) const;
	int pp(int n) const;

	//about level-up
	int expCurve(int level) const;

	//virtual methods
	virtual bool attack(Pokemon *attacker, Pokemon *aim, int skillIndex = 0) = 0;
};

class Race_1 : public PokemonBase
{
public:
	Race_1();
	bool attack(Pokemon *attacker, Pokemon *aim, int skillIndex = 0);
};

class Race_2 : public PokemonBase
{
public:
	Race_2();
	bool attack(Pokemon *attacker, Pokemon *aim, int skillIndex = 0);
};

class Pokemon
{
private:
	PokemonBase *_race;
	string _name;
	int _atk;
	int _def;
	int _hp;
	int _maxHp;
	int _speed;
	int _lv; //level
	int _exp;
	int _skillPriority[4];
	int _pp[3]; //power point

public:
	Pokemon(PokemonBase *race = NULL, const string &name = "");

	//getter
	string name() const { return _name; }
	int atk() const { return _atk; }
	int def() const { return _def; }
	int hp() const { return _hp; }
	int speed() const { return _speed; }
	int lv() const { return _lv; }
	int exp() const { return _exp; }
	//race getter
	string raceName() const { return _race->raceName(); }
	string raceType() const;
	//setter
	void changeAtk(int count);
	void changeDef(int count);
	void changeSpeed(int count);
	bool changeHp(int count); //return true if hp = 0

	//about battle
	bool attack(Pokemon *aim, int skillIndex = 0) { return _race->attack(this, aim, skillIndex); }
	bool getExp(int count); //return true if level-up
};