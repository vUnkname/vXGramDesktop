// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

namespace Ui {
class GenericBox;
} // namespace Ui

namespace AyuUi {

// Model picker for the "AI Translator": type a model name or choose one
// of the models fetched from the provider. "Test" sends a tiny request.
void LlmModelBox(not_null<Ui::GenericBox*> box, int preset);

} // namespace AyuUi
