#include "uasset.h"

/*
 * Changes to how the creator saves songs, applied to every MIDI resource it loads or imports,
 * so songs saved by older versions match what this version writes.
 *
 * Saved songs don't record which creator version wrote them, so every migration runs on every
 * load and has to leave data that is already current unchanged.
 */
namespace {
	using MidiFileResource = HmxAudio::PackageFile::MidiFileResource;

	// bc3df7d renamed the "b2" chord option to "1sus2" without converting songs saved before it
	void renameB2Chords(MidiFileResource& midi) {
		for (auto& chord : midi.chords) {
			if (chord.name == "b2") {
				chord.name = "1sus2";
			}
		}
	}

	void (*const migrations[])(MidiFileResource&) = {
		renameB2Chords,
	};
}

void HmxAudio::PackageFile::MidiFileResource::applyMigrations() {
	for (auto migrate : migrations) {
		migrate(*this);
	}
}
