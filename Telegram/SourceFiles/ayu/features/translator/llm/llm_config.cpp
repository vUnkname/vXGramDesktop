// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/llm/llm_config.h"

#include "ayu/ayu_settings.h"
#include "ayu/features/translator/llm/llm_model_util.h"
#include "ayu/features/translator/llm/llm_presets.h"

#include <rpl/flatten_latest.h>
#include <rpl/map.h>
#include <rpl/single.h>

#include <iterator>

namespace Ayu::Translator::Llm::Config {
namespace {

[[nodiscard]] AyuSettings &Settings() {
	return AyuSettings::getInstance();
}

constexpr auto kDefaultCustomUrl = "https://generativelanguage.googleapis.com/v1beta/openai";

} // namespace

int CurrentPreset() {
	const auto preset = Settings().llmProviderPreset();
	return ((preset >= 0) && (preset < kPresetCount)) ? preset : 0;
}

QString SavedModel(int preset) {
	auto &s = Settings();
	switch (preset) {
	case 0: return s.llmPresetModel0().trimmed();
	case 1: return s.llmPresetModel1().trimmed();
	case 2: return s.llmPresetModel2().trimmed();
	case 3: return s.llmPresetModel3().trimmed();
	case 4: return s.llmPresetModel4().trimmed();
	case 5: return s.llmPresetModel5().trimmed();
	case 6: return s.llmPresetModel6().trimmed();
	case 7: return s.llmPresetModel7().trimmed();
	case 8: return s.llmPresetModel8().trimmed();
	case 9: return s.llmPresetModel9().trimmed();
	}
	return {};
}

void SetSavedModel(int preset, const QString &model) {
	auto &s = Settings();
	const auto value = model.trimmed();
	switch (preset) {
	case 0: s.setLlmPresetModel0(value); break;
	case 1: s.setLlmPresetModel1(value); break;
	case 2: s.setLlmPresetModel2(value); break;
	case 3: s.setLlmPresetModel3(value); break;
	case 4: s.setLlmPresetModel4(value); break;
	case 5: s.setLlmPresetModel5(value); break;
	case 6: s.setLlmPresetModel6(value); break;
	case 7: s.setLlmPresetModel7(value); break;
	case 8: s.setLlmPresetModel8(value); break;
	case 9: s.setLlmPresetModel9(value); break;
	}
}

QString EffectiveModel(int preset) {
	const auto saved = SavedModel(preset);
	return saved.isEmpty() ? PresetDefaultModel(preset) : saved;
}

rpl::producer<QString> EffectiveModelValue(int preset) {
	auto &s = Settings();
	const auto map = [=](const QString &saved) {
		const auto trimmed = saved.trimmed();
		return trimmed.isEmpty() ? PresetDefaultModel(preset) : trimmed;
	};
	switch (preset) {
	case 0: return s.llmPresetModel0Value() | rpl::map(map);
	case 1: return s.llmPresetModel1Value() | rpl::map(map);
	case 2: return s.llmPresetModel2Value() | rpl::map(map);
	case 3: return s.llmPresetModel3Value() | rpl::map(map);
	case 4: return s.llmPresetModel4Value() | rpl::map(map);
	case 5: return s.llmPresetModel5Value() | rpl::map(map);
	case 6: return s.llmPresetModel6Value() | rpl::map(map);
	case 7: return s.llmPresetModel7Value() | rpl::map(map);
	case 8: return s.llmPresetModel8Value() | rpl::map(map);
	case 9: return s.llmPresetModel9Value() | rpl::map(map);
	}
	return rpl::single(QString());
}

rpl::producer<QString> CurrentEffectiveModelValue() {
	return Settings().llmProviderPresetValue(
	) | rpl::map([](int preset) {
		return ((preset >= 0) && (preset < kPresetCount)) ? preset : 0;
	}) | rpl::map([](int preset) {
		return EffectiveModelValue(preset);
	}) | rpl::flatten_latest();
}

QString EffectiveBaseUrl(int preset) {
	if (preset == Custom) {
		const auto user = Settings().llmApiUrl().trimmed();
		return user.isEmpty()
			? QString::fromLatin1(kDefaultCustomUrl)
			: user;
	}
	return PresetBaseUrl(preset);
}

void SetCustomBaseUrl(const QString &url) {
	Settings().setLlmApiUrl(ModelUtil::NormalizeBaseUrl(url));
}

QString ApiKeys(int preset) {
	auto &s = Settings();
	switch (preset) {
	case 0: return s.llmApiKey();
	case 1: return s.llmPresetKey1();
	case 2: return s.llmPresetKey2();
	case 3: return s.llmPresetKey3();
	case 4: return s.llmPresetKey4();
	case 5: return s.llmPresetKey5();
	case 6: return s.llmPresetKey6();
	case 7: return s.llmPresetKey7();
	case 8: return s.llmPresetKey8();
	case 9: return s.llmPresetKey9();
	}
	return {};
}

void SetApiKeys(int preset, const QString &keys) {
	auto &s = Settings();
	const auto value = keys.trimmed();
	switch (preset) {
	case 0: s.setLlmApiKey(value); break;
	case 1: s.setLlmPresetKey1(value); break;
	case 2: s.setLlmPresetKey2(value); break;
	case 3: s.setLlmPresetKey3(value); break;
	case 4: s.setLlmPresetKey4(value); break;
	case 5: s.setLlmPresetKey5(value); break;
	case 6: s.setLlmPresetKey6(value); break;
	case 7: s.setLlmPresetKey7(value); break;
	case 8: s.setLlmPresetKey8(value); break;
	case 9: s.setLlmPresetKey9(value); break;
	}
}

rpl::producer<QString> ApiKeysValue(int preset) {
	auto &s = Settings();
	switch (preset) {
	case 0: return s.llmApiKeyValue();
	case 1: return s.llmPresetKey1Value();
	case 2: return s.llmPresetKey2Value();
	case 3: return s.llmPresetKey3Value();
	case 4: return s.llmPresetKey4Value();
	case 5: return s.llmPresetKey5Value();
	case 6: return s.llmPresetKey6Value();
	case 7: return s.llmPresetKey7Value();
	case 8: return s.llmPresetKey8Value();
	case 9: return s.llmPresetKey9Value();
	}
	return rpl::single(QString());
}

QStringList ApiKeyList(int preset) {
	auto result = QStringList();
	for (const auto &part : ApiKeys(preset).split(',')) {
		const auto key = part.trimmed();
		if (!key.isEmpty() && !result.contains(key)) {
			result.push_back(key);
		}
	}
	return result;
}

QString FirstApiKey(int preset) {
	const auto list = ApiKeyList(preset);
	return list.isEmpty() ? QString() : list.front();
}

bool IsAvailable() {
	return !ApiKeys(CurrentPreset()).trimmed().isEmpty();
}

int ContextMessageLimit() {
	static constexpr int kOptions[] = { 1, 3, 5, 7, 10 };
	const auto index = Settings().llmContextSize();
	return ((index >= 0) && (index < int(std::size(kOptions))))
		? kOptions[index]
		: 5;
}

} // namespace Ayu::Translator::Llm::Config
