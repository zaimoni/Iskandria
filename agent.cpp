// agent.cpp

#include "agent.hpp"
#include "civilization.hpp"

#include "world_manager.hpp"

namespace iskandria {

#define HUMAN "Terran"
#define HUMAN_DESC "Men and women.  No surprises here."

#define BADGER "Badger"
#define BADGER_DESC "Iskandran Badger.  Similiarites to Eurasian badgers are cosmetic."

#define WYLKRAI "Wylkrai"
#define WYLKRAI_DESC "Similarities to tigers are cosmetic."

#define TZARZ "Tzarz"
#define TZARZ_DESC "Arachnoreptilian.  Has as many compound eyes, and arms, as needed for his job."

#define HUMAN_ROBOT "Robot"
#define HUMAN_ROBOT_DESC "Clearly built with Terran technology."

#define HUMAN_TELEPRESENCE "Remote-controlled bot"
#define HUMAN_TELEPRESENCE_DESC "The Terran operator is elsewhere, with an acceptably low speed of light lag."

#define BADGER_ROBOT "Robot"
#define BADGER_ROBOT_DESC "Clearly built with Iskandran technology."

#define BADGER_TELEPRESENCE "Remote-controlled bot"
#define BADGER_TELEPRESENCE_DESC "The Iskandran operator is elsewhere, with an acceptably low speed of light lag."

#define BADGER_TELEPRESENCE_IT "Remote-controlled bot"
#define BADGER_TELEPRESENCE_IT_DESC "The Iskandran operator is plausibly several light-years elsewhere."

#define BADGER_BADGERIFORM_ROBOT "Badgeriform Robot"
#define BADGER_BADGERIFORM_ROBOT_DESC "Can use any control system an Iskandran Badger can use."

#define BADGER_BADGERIFORM_HOLOGRAM "Badgeriform Force Feedback Hologram"
#define BADGER_BADGERIFORM_HOLOGRAM_DESC "Must be burning power at an astronomical rate."

#define ZAIMONIHOME_HOLOGRAM "Force Feedback Hologram"
#define ZAIMONIHOME_HOLOGRAM_DESC "Material objects only think (s)he's there."

#define WYLKRAI_ROBOT "Robot"
#define WYLKRAI_ROBOT_DESC "Clearly built with Wylkrai technology."

#define WYLKRAI_TELEPRESENCE "Remote-controlled bot"
#define WYLKRAI_TELEPRESENCE_DESC "The Wylkrai operator is elsewhere, with an acceptably low speed of light lag."

#define TZARZ_ROBOT "Robot"
#define TZARZ_ROBOT_DESC "Clearly built with Tzarz technology."

#define TZARZ_TELEPRESENCE "Remote-controlled bot"
#define TZARZ_TELEPRESENCE_DESC "The Tzarz operator is elsewhere, with an acceptably low speed of light lag."

static const agent_species _species[16] = {
	{CIV_TERRAN, HUMAN, sizeof(HUMAN)-1, HUMAN_DESC, sizeof(HUMAN_DESC)-1},
	{CIV_ISKANDRAN, BADGER, sizeof(BADGER)-1, BADGER_DESC, sizeof(BADGER_DESC)-1},
	{CIV_WYLKRAI, WYLKRAI, sizeof(WYLKRAI)-1, WYLKRAI_DESC, sizeof(WYLKRAI_DESC)-1},
	{CIV_TZARZ, TZARZ, sizeof(TZARZ)-1, TZARZ_DESC, sizeof(TZARZ_DESC)-1},
	{CIV_TERRAN, HUMAN_ROBOT, sizeof(HUMAN_ROBOT)-1, HUMAN_ROBOT_DESC, sizeof(HUMAN_ROBOT_DESC)-1},
	{CIV_ISKANDRAN, BADGER_ROBOT, sizeof(BADGER_ROBOT)-1, BADGER_ROBOT_DESC, sizeof(BADGER_ROBOT_DESC)-1},
	{CIV_WYLKRAI, WYLKRAI_ROBOT, sizeof(WYLKRAI_ROBOT)-1, WYLKRAI_ROBOT_DESC, sizeof(WYLKRAI_ROBOT_DESC)-1},
	{CIV_TZARZ, TZARZ_ROBOT, sizeof(TZARZ_ROBOT)-1, TZARZ_ROBOT_DESC, sizeof(TZARZ_ROBOT_DESC)-1},
	{CIV_TERRAN, HUMAN_TELEPRESENCE, sizeof(HUMAN_TELEPRESENCE)-1, HUMAN_TELEPRESENCE_DESC, sizeof(HUMAN_TELEPRESENCE_DESC)-1},
	{CIV_ISKANDRAN, BADGER_TELEPRESENCE, sizeof(BADGER_TELEPRESENCE)-1, BADGER_TELEPRESENCE_DESC, sizeof(BADGER_TELEPRESENCE_DESC)-1},
	{CIV_WYLKRAI, WYLKRAI_TELEPRESENCE, sizeof(WYLKRAI_TELEPRESENCE)-1, WYLKRAI_TELEPRESENCE_DESC, sizeof(WYLKRAI_TELEPRESENCE_DESC)-1},
	{CIV_TZARZ, TZARZ_TELEPRESENCE, sizeof(TZARZ_TELEPRESENCE)-1, TZARZ_TELEPRESENCE_DESC, sizeof(TZARZ_TELEPRESENCE_DESC)-1},
	{CIV_ISKANDRAN, BADGER_TELEPRESENCE_IT, sizeof(BADGER_TELEPRESENCE_IT)-1, BADGER_TELEPRESENCE_IT_DESC, sizeof(BADGER_TELEPRESENCE_IT_DESC)-1},
	{CIV_ISKANDRAN, BADGER_BADGERIFORM_ROBOT, sizeof(BADGER_BADGERIFORM_ROBOT)-1, BADGER_BADGERIFORM_ROBOT_DESC, sizeof(BADGER_BADGERIFORM_ROBOT_DESC)-1},
	{CIV_ISKANDRAN, BADGER_BADGERIFORM_HOLOGRAM, sizeof(BADGER_BADGERIFORM_HOLOGRAM)-1, BADGER_BADGERIFORM_HOLOGRAM_DESC, sizeof(BADGER_BADGERIFORM_HOLOGRAM_DESC)-1},
	{CIV_ZAIMONIHOME, ZAIMONIHOME_HOLOGRAM, sizeof(ZAIMONIHOME_HOLOGRAM)-1, ZAIMONIHOME_HOLOGRAM_DESC, sizeof(ZAIMONIHOME_HOLOGRAM_DESC)-1}
};

const agent_species* agent::species = _species;
const size_t agent::species_len = STATIC_SIZE(_species);

agent::agent(FILE* src)
{
	_species_index = zaimoni::read<decltype(_species_index)>(src,species_len);
}

void agent::save(FILE* dest)
{
	zaimoni::write(_species_index,dest,species_len);
}


std::vector<std::shared_ptr<agent> >& agent::cache()
{
	static std::vector<std::shared_ptr<agent> > ooao;
	return ooao;
}

std::weak_ptr<agent> agent::track(std::shared_ptr<agent> src)
{
	if (!src.get()) return std::weak_ptr<agent>();
	cache().push_back(src);
	return src;
}

void agent::world_setup()
{
	isk::WorldManager& cosmos = isk::WorldManager::get();
	cosmos.register_update(update_all);
	cosmos.register_gc(gc_all);
	cosmos.register_load(load_all);
	cosmos.register_save(save_all);
}

std::weak_ptr<agent> agent::read_synthetic_id(FILE* src)
{
	return isk::Object::read_synthetic_id(cache(), src);
}

void agent::write_synthetic_id(const std::shared_ptr<agent>& src,FILE* dest)
{
	isk::Object::write_synthetic_id(cache().size(), src, dest);
}

void agent::gc_all()	// XXX could go in header, but this *has* to exist enough to give a function pointer
{
	isk::Object::gc_all(cache());
}

void agent::load_all(FILE* src)
{
	size_t tmp;
	ZAIMONI_FREAD_OR_DIE(size_t,tmp,src)
	if (0==tmp) {
		cache().clear();
		return;
	}
	std::vector<std::shared_ptr<agent> > dest(tmp);
	while(0 < tmp--) dest.push_back(std::shared_ptr<agent>(new agent(src)));
	swap(dest,cache());
}


void agent::save_all(FILE* dest)
{
	gc_all();
	isk::Object::init_synthetic_ids(cache());
	size_t tmp = cache().size();
	ZAIMONI_FWRITE_OR_DIE(size_t,tmp,dest)
	for(auto i : cache()) i->save(dest);
}

void agent::update_all()
{
	std::vector<std::shared_ptr<agent> > staging;
	for(auto i : cache()) {
		agent* tmp = i.get();
		if (!tmp) continue;
		// just because it's going away doesn't mean it can't do things
		// do not insert into cache() here, insert to staging instead
	}
	if (!staging.empty()) cache().insert(cache().end(),staging.begin(),staging.end());
}

}	// namespace iskandria

#ifdef TEST_APP2
// fast compile test
// g++ -std=c++11 -otest.exe -Os -DTEST_APP2 -D__STDC_LIMIT_MACROS agent.cpp world_manager.cpp -Llib\host.isk -lz_stdio_c -lz_stdio_log
int main(int argc, char* argv[])
{
	iskandria::agent::world_setup();
	return 0;
}
#endif
