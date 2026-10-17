#include "uasset.h"
#include "version.h"

#include <iterator>

/*
 * Changes to how the creator saves songs. Each migration names the save format that made its
 * change, and runs only on data saved in an older format: songs record theirs as
 * CreatorSaveFormat, and songs from before that stamp, or re-saved by a creator from before it,
 * count as format 0, as do imported MIDIs.
 *
 * A new migration takes the next format number; songs saved from then on record it, so they
 * never need the migration.
 *
 * A re-save by a creator from before the stamp is only detected through the song's chords (see
 * AssetRoot::hasPreChordFixEnds). A song without chords keeps its stale stamp, so a migration
 * that isn't about chords can be skipped on such a song.
 */
namespace {
	using MidiFileResource = HmxAudio::PackageFile::MidiFileResource;

	struct Migration {
		int format;
		void (*apply)(MidiFileResource&);
	};

	// bc3df7d renamed the "b2" chord option to "1sus2" without converting songs saved before it
	void renameB2Chords(MidiFileResource& midi) {
		for (auto& chord : midi.chords) {
			if (chord.name == "b2") {
				chord.name = "1sus2";
			}
		}
	}

	constexpr Migration migrations[] = {
		{ 1, renameB2Chords },
	};

	constexpr bool formatsAreContiguous() {
		for (size_t i = 0; i < std::size(migrations); i++) {
			if (migrations[i].format != int(i) + 1) {
				return false;
			}
		}
		return true;
	}

	/*
	 * Songs store the format they were saved with, so removing, reordering or inserting an entry
	 * would change which migrations already-saved songs skip. To retire a migration, make its
	 * function a no-op and keep its entry.
	 */
	static_assert(formatsAreContiguous(), "migrations must be numbered 1, 2, 3... in order");
}

// A constant expression, so it is set at compile time rather than during static initialization
extern const int CurrentSaveFormat = migrations[std::size(migrations) - 1].format;

void HmxAudio::PackageFile::MidiFileResource::applyMigrations(int savedFormat) {
	for (auto& migration : migrations) {
		if (savedFormat < migration.format) {
			migration.apply(*this);
		}
	}
}
