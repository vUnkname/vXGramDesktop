// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/ui/settings/settings_translator.h"

#include "lang_auto.h"
#include "ayu/ayu_settings.h"
#include "ayu/features/translator/llm/llm_config.h"
#include "ayu/features/translator/llm/llm_model_util.h"
#include "ayu/features/translator/llm/llm_presets.h"
#include "ayu/features/translator/translator_providers.h"
#include "ayu/ui/boxes/llm_model_box.h"
#include "ayu/ui/settings/ayu_builder.h"
#include "ayu/ui/settings/settings_main.h"
#include "ayu/ui/settings/translator/translator_settings_utils.h"
#include "base/platform/base_platform_info.h"
#include "boxes/translate_box.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "spellcheck/spellcheck_types.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/boxes/choose_language_box.h"
#include "ui/boxes/single_choice_box.h"
#include "ui/layers/generic_box.h"
#include "ui/toast/toast.h"
#include "ui/ui_utility.h"
#include <rpl/flatten_latest.h>
#include <cmath>
#include "ui/widgets/buttons.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

namespace Settings {

using namespace Builder;
using namespace AyuBuilder;
namespace Llm = Ayu::Translator::Llm;

namespace {

void BuildOptions(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(tr::ayu_TranslatorOptionsHeader());

	const auto showButton = builder.addButton({
		.id = u"ayu/translator/showButton"_q,
		.title = tr::lng_translate_settings_show(),
		.st = &st::settingsButtonNoIcon,
		.toggled = rpl::single(
			Core::App().settings().translateButtonEnabled()),
	});
	if (showButton) {
		showButton->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().translateButtonEnabled());
		}) | rpl::on_next([](bool checked) {
			Core::App().settings().setTranslateButtonEnabled(checked);
			Core::App().saveSettingsDelayed();
		}, showButton->lifetime());
	}

	const auto translateChats = builder.addButton({
		.id = u"ayu/translator/translateChats"_q,
		.title = tr::lng_translate_settings_chat(),
		.st = &st::settingsButtonNoIcon,
		.toggled = Core::App().settings().translateChatEnabledValue(),
	});
	if (translateChats) {
		translateChats->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().translateChatEnabled());
		}) | rpl::on_next([](bool checked) {
			Core::App().settings().setTranslateChatEnabled(checked);
			Core::App().saveSettingsDelayed();
		}, translateChats->lifetime());
	}

	ayu.addSettingToggle({
		.id = u"ayu/translator/keepMarkdown"_q,
		.title = tr::ayu_TranslatorKeepMarkdown(),
		.getter = &AyuSettings::translatorKeepMarkdown,
		.setter = &AyuSettings::setTranslatorKeepMarkdown,
	});

	builder.addSkip();
	builder.addDividerText(tr::lng_translate_settings_about());
}

struct ProviderButtonArgs {
	QString id;
	rpl::producer<QString> title;
	bool forArticles = false;
	Fn<TranslationProvider()> getter;
	rpl::producer<TranslationProvider> value;
	Fn<void(TranslationProvider)> setter;
	rpl::producer<bool> shown;
};

void AddProviderButton(SectionBuilder &builder, ProviderButtonArgs &&args) {
	const auto controller = builder.controller();
	const auto forArticles = args.forArticles;
	const auto getter = args.getter;
	const auto setter = args.setter;

	auto label = std::move(args.value) | rpl::map([](
			TranslationProvider provider) {
		return Ayu::Translator::ProviderName(provider);
	});

	builder.addButton({
		.id = args.id,
		.title = rpl::duplicate(args.title),
		.st = &st::settingsButtonNoIcon,
		.label = std::move(label),
		.onClick = [=] {
			if (!controller) {
				return;
			}
			controller->show(Box([=](not_null<Ui::GenericBox*> box) {
				const auto entries
					= Ayu::Translator::AvailableProviders(forArticles);
				auto labels = std::vector<QString>();
				auto selected = 0;
				const auto current = getter();
				for (auto i = 0; i != int(entries.size()); ++i) {
					labels.push_back(entries[i].name);
					if (entries[i].provider == current) {
						selected = i;
					}
				}
				const auto save = [=](int index) {
					const auto provider = entries[index].provider;
					setter(provider);
					if constexpr (Platform::IsMac()) {
						if (provider == TranslationProvider::Native) {
							controller->showToast(Ui::Toast::Config{
								.text = tr::lng_translate_settings_use_platform_mac_about(
									tr::now,
									tr::rich),
								.duration = 6 * crl::time(1000),
							});
						}
					}
				};
				SingleChoiceBox(box, {
					.title = tr::ayu_TranslationProvider(),
					.options = labels,
					.initialSelection = selected,
					.callback = save,
				});
			}));
		},
		.shown = std::move(args.shown),
	});
}

void BuildProvider(SectionBuilder &builder) {
	auto &settings = AyuSettings::getInstance();
	AddProviderButton(builder, ProviderButtonArgs{
		.id = u"ayu/translationProvider"_q,
		.title = tr::ayu_TranslationProvider(),
		.getter = [] {
			return AyuSettings::getInstance().translationProvider();
		},
		.value = settings.translationProviderValue(),
		.setter = [](TranslationProvider provider) {
			AyuSettings::getInstance().setTranslationProvider(provider);
		},
	});
}

void BuildTargetLanguage(SectionBuilder &builder) {
	const auto controller = builder.controller();

	const auto followApp = builder.addButton({
		.id = u"ayu/translator/followApp"_q,
		.title = tr::ayu_TranslatorFollowApp(),
		.st = &st::settingsButtonNoIcon,
		.toggled = Core::App().settings().translateToFollowsAppValue(),
	});
	if (followApp) {
		followApp->toggledValue(
		) | rpl::filter([](bool checked) {
			return (checked != Core::App().settings().translateToFollowsApp());
		}) | rpl::on_next([](bool checked) {
			auto &settings = Core::App().settings();
			settings.setTranslateTo(checked
				? LanguageId()
				: settings.translateTo());
			Core::App().saveSettingsDelayed();
		}, followApp->lifetime());
	}

	builder.addButton({
		.id = u"ayu/translator/targetLanguage"_q,
		.title = tr::ayu_TranslatorTargetLanguage(),
		.st = &st::settingsButtonNoIcon,
		.label = Core::App().settings().translateToValue(
		) | rpl::map([](LanguageId id) {
			return Ui::LanguageName(id);
		}),
		.onClick = [=] {
			if (!controller) {
				return;
			}
			controller->show(Ui::ChooseTranslateToBox(
				Core::App().settings().translateTo(),
				[](LanguageId id) {
					Core::App().settings().setTranslateTo(id);
					Core::App().saveSettingsDelayed();
				}));
		},
		.shown = Core::App().settings().translateToFollowsAppValue(
		) | rpl::map([](bool follows) {
			return !follows;
		}),
	});
}

void BuildDoNotTranslate(SectionBuilder &builder) {
	const auto controller = builder.controller();

	using Languages = std::vector<LanguageId>;
	builder.addButton({
		.id = u"ayu/translator/doNotTranslate"_q,
		.title = tr::lng_translate_settings_choose(),
		.st = &st::settingsButtonNoIcon,
		.label = Core::App().settings().skipTranslationLanguagesValue(
		) | rpl::map([](const Languages &list) {
			return (list.size() > 1)
				? tr::lng_languages_count(tr::now, lt_count, list.size())
				: Ui::LanguageName(list.front());
		}),
		.onClick = [=] {
			if (controller) {
				controller->show(Ui::EditSkipTranslationLanguages());
			}
		},
	});
}

void BuildTranslate(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	using namespace TranslatorUi;
	auto *settings = &AyuSettings::getInstance();

	builder.addSubsectionTitle(tr::ayu_TranslatorTranslateHeader());

	BuildProvider(builder);

	const auto originalOptions = std::vector{
		tr::ayu_TranslatorWithOriginalOff(tr::now),
		tr::ayu_TranslatorWithOriginalManual(tr::now),
		tr::ayu_TranslatorWithOriginalOn(tr::now),
	};
	ayu.addChooseButton({
		.id = u"ayu/translator/withOriginal"_q,
		.title = tr::ayu_TranslatorWithOriginal(),
		.boxTitle = tr::ayu_TranslatorWithOriginal(),
		.initialSelection = std::clamp(
			settings->translatorWithOriginalText(),
			0,
			int(originalOptions.size()) - 1),
		.options = originalOptions,
		.setter = [](int i) {
			AyuSettings::getInstance().setTranslatorWithOriginalText(i);
		},
	});

	BuildTargetLanguage(builder);
	BuildDoNotTranslate(builder);

	AddTextValueButton(builder, TextValueArgs{
		.id = u"ayu/translator/preferredLanguages"_q,
		.title = tr::ayu_TranslatorPreferredLanguages(),
		.boxTitle = tr::ayu_TranslatorPreferredLanguages(tr::now),
		.placeholder = tr::ayu_TranslatorPreferredLanguagesPlaceholder(tr::now),
		.description = tr::ayu_TranslatorPreferredLanguagesNotice(tr::now),
		.value = settings->preferredTranslateTargetLangValue(),
		.getter = [] {
			return AyuSettings::getInstance().preferredTranslateTargetLang();
		},
		.setter = [](const QString &value) {
			AyuSettings::getInstance().setPreferredTranslateTargetLang(value);
		},
	});

	AddTextValueButton(builder, TextValueArgs{
		.id = u"ayu/translator/googleCloudKey"_q,
		.title = tr::ayu_TranslatorGoogleCloudKey(),
		.boxTitle = tr::ayu_TranslatorGoogleCloudKey(tr::now),
		.placeholder = tr::ayu_TranslatorApiKeyPlaceholder(tr::now),
		.description = tr::ayu_TranslatorKeyNotice(tr::now),
		.helpUrl = u"https://console.cloud.google.com/apis/credentials"_q,
		.secret = true,
		.value = settings->googleCloudTranslateKeyValue(),
		.getter = [] {
			return AyuSettings::getInstance().googleCloudTranslateKey();
		},
		.setter = [](const QString &value) {
			AyuSettings::getInstance().setGoogleCloudTranslateKey(value);
		},
		.shown = rpl::combine(
			settings->translationProviderValue(),
			settings->googleTranslateExpValue(),
			[](TranslationProvider provider, bool experimental) {
				return (provider == TranslationProvider::Google)
					&& !experimental;
			}),
	});

	AddTextValueButton(builder, TextValueArgs{
		.id = u"ayu/translator/deepLKey"_q,
		.title = tr::ayu_TranslatorDeepLKey(),
		.boxTitle = tr::ayu_TranslatorDeepLKey(tr::now),
		.placeholder = tr::ayu_TranslatorApiKeyPlaceholder(tr::now),
		.description = tr::ayu_TranslatorKeyNotice(tr::now),
		.helpUrl = u"https://www.deepl.com/your-account/keys"_q,
		.secret = true,
		.value = settings->deepLTranslateKeyValue(),
		.getter = [] {
			return AyuSettings::getInstance().deepLTranslateKey();
		},
		.setter = [](const QString &value) {
			AyuSettings::getInstance().setDeepLTranslateKey(value);
		},
		.shown = settings->translationProviderValue(
		) | rpl::map([](TranslationProvider provider) {
			return (provider == TranslationProvider::DeepL);
		}),
	});
}

[[nodiscard]] rpl::producer<int> PresetValue() {
	return AyuSettings::getInstance().llmProviderPresetValue(
	) | rpl::map([](int preset) {
		return ((preset >= 0) && (preset < Llm::kPresetCount)) ? preset : 0;
	});
}

void BuildAiTranslator(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	using namespace TranslatorUi;
	auto *settings = &AyuSettings::getInstance();
	const auto controller = builder.controller();

	builder.addSubsectionTitle(tr::ayu_TranslatorAiHeader());

	// Model provider preset.
	builder.addButton({
		.id = u"ayu/translator/llmPreset"_q,
		.title = tr::ayu_LlmProviderPreset(),
		.st = &st::settingsButtonNoIcon,
		.label = PresetValue() | rpl::map([](int preset) {
			return Llm::PresetName(preset);
		}),
		.onClick = [=] {
			if (!controller) {
				return;
			}
			controller->show(Box([=](not_null<Ui::GenericBox*> box) {
				auto labels = std::vector<QString>();
				for (auto i = 0; i != Llm::kPresetCount; ++i) {
					labels.push_back(Llm::PresetName(i));
				}
				SingleChoiceBox(box, {
					.title = tr::ayu_LlmProviderPreset(),
					.options = labels,
					.initialSelection = Llm::Config::CurrentPreset(),
					.callback = [](int index) {
						AyuSettings::getInstance().setLlmProviderPreset(
							index);
					},
				});
			}));
		},
	});

	// Model.
	builder.addButton({
		.id = u"ayu/translator/llmModel"_q,
		.title = tr::ayu_LlmModelName(),
		.st = &st::settingsButtonNoIcon,
		.label = Llm::Config::CurrentEffectiveModelValue(),
		.onClick = [=] {
			if (!controller) {
				return;
			}
			const auto preset = Llm::Config::CurrentPreset();
			controller->show(Box([=](not_null<Ui::GenericBox*> box) {
				AyuUi::LlmModelBox(box, preset);
			}));
		},
	});

	// API key of the selected preset.
	AddTextValueButton(builder, TextValueArgs{
		.id = u"ayu/translator/llmApiKey"_q,
		.title = tr::ayu_LlmApiKey(),
		.boxTitle = tr::ayu_LlmApiKey(tr::now),
		.placeholder = tr::ayu_LlmApiKey(tr::now),
		.description = tr::ayu_LlmApiKeyNotice(tr::now),
		.helpUrlGetter = [] {
			return Llm::PresetKeyUrl(Llm::Config::CurrentPreset());
		},
		.secret = true,
		.value = PresetValue() | rpl::map([](int preset) {
			return Llm::Config::ApiKeysValue(preset);
		}) | rpl::flatten_latest(),
		.getter = [] {
			return Llm::Config::ApiKeys(Llm::Config::CurrentPreset());
		},
		.setter = [](const QString &value) {
			Llm::Config::SetApiKeys(Llm::Config::CurrentPreset(), value);
		},
	});

	// Custom endpoint URL.
	AddTextValueButton(builder, TextValueArgs{
		.id = u"ayu/translator/llmApiUrl"_q,
		.title = tr::ayu_LlmApiUrl(),
		.boxTitle = tr::ayu_LlmApiUrl(tr::now),
		.placeholder = tr::ayu_LlmApiUrlHint(tr::now),
		.description = tr::ayu_LlmApiUrlNotice(tr::now),
		.showReset = true,
		.emptyLabel = Llm::Config::EffectiveBaseUrl(Llm::Custom),
		.value = settings->llmApiUrlValue(),
		.getter = [] {
			return AyuSettings::getInstance().llmApiUrl();
		},
		.setter = [](const QString &value) {
			Llm::Config::SetCustomBaseUrl(value);
		},
		.validate = [](const QString &value) {
			return Llm::ModelUtil::IsValidBaseUrl(value);
		},
		.shown = PresetValue() | rpl::map([](int preset) {
			return (preset == Llm::Custom);
		}),
	});

	// Prompts.
	AddTextValueButton(builder, TextValueArgs{
		.id = u"ayu/translator/llmSystemPrompt"_q,
		.title = tr::ayu_LlmSystemPrompt(),
		.boxTitle = tr::ayu_LlmSystemPrompt(tr::now),
		.placeholder = tr::ayu_LlmSystemPromptHint(tr::now),
		.description = tr::ayu_LlmSystemPromptNotice(tr::now),
		.multiline = true,
		.showReset = true,
		.emptyLabel = tr::ayu_LlmDefault(tr::now),
		.value = settings->llmSystemPromptValue(),
		.getter = [] {
			return AyuSettings::getInstance().llmSystemPrompt();
		},
		.setter = [](const QString &value) {
			AyuSettings::getInstance().setLlmSystemPrompt(value);
		},
	});
	AddTextValueButton(builder, TextValueArgs{
		.id = u"ayu/translator/llmUserPrompt"_q,
		.title = tr::ayu_LlmUserPrompt(),
		.boxTitle = tr::ayu_LlmUserPrompt(tr::now),
		.placeholder = tr::ayu_LlmUserPromptHint(tr::now),
		.description = tr::ayu_LlmUserPromptNotice(tr::now),
		.multiline = true,
		.showReset = true,
		.emptyLabel = tr::ayu_LlmDefault(tr::now),
		.value = settings->llmUserPromptValue(),
		.getter = [] {
			return AyuSettings::getInstance().llmUserPrompt();
		},
		.setter = [](const QString &value) {
			AyuSettings::getInstance().setLlmUserPrompt(value);
		},
	});

	// Context.
	ayu.addSettingToggle({
		.id = u"ayu/translator/llmUseContext"_q,
		.title = tr::ayu_LlmUseContext(),
		.getter = &AyuSettings::llmUseContext,
		.setter = &AyuSettings::setLlmUseContext,
	});
	builder.scope([&] {
		ayu.addSettingToggle({
			.id = u"ayu/translator/llmUseContextInAutoTranslate"_q,
			.title = tr::ayu_LlmUseContextInAutoTranslate(),
			.getter = &AyuSettings::llmUseContextInAutoTranslate,
			.setter = &AyuSettings::setLlmUseContextInAutoTranslate,
		});
		ayu.addChooseButton({
			.id = u"ayu/translator/llmContextSize"_q,
			.title = tr::ayu_LlmContextSize(),
			.boxTitle = tr::ayu_LlmContextSize(),
			.initialSelection = std::clamp(
				AyuSettings::getInstance().llmContextSize(),
				0,
				4),
			.options = {
				u"1"_q, u"3"_q, u"5"_q, u"7"_q, u"10"_q
			},
			.setter = [](int index) {
				AyuSettings::getInstance().setLlmContextSize(index);
			},
		});
	}, settings->llmUseContextValue());

	// Temperature (only for models which support it).
	ayu.addSlider({
		.id = u"ayu/translator/llmTemperature"_q,
		.title = tr::ayu_LlmTemperature(),
		.steps = 21,
		.current = int(std::round(settings->llmTemperature() * 10.)),
		.indexToValue = [](int index) { return index; },
		.onFinalChanged = [](int index) {
			AyuSettings::getInstance().setLlmTemperature(index / 10.);
		},
		.formatLabel = [](int index) {
			return QString::number(index / 10., 'f', 1);
		},
		.shown = Llm::Config::CurrentEffectiveModelValue(
		) | rpl::map([](const QString &model) {
			return Llm::ModelUtil::SupportsTemperature(model);
		}),
	});
}

void BuildArticle(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	auto &settings = AyuSettings::getInstance();

	builder.addSubsectionTitle(tr::ayu_TranslatorArticleHeader());

	ayu.addSettingToggle({
		.id = u"ayu/translator/separateArticle"_q,
		.title = tr::ayu_TranslatorSeparateArticle(),
		.getter = &AyuSettings::enableSeparateArticleTranslator,
		.setter = &AyuSettings::setEnableSeparateArticleTranslator,
	});
	AddProviderButton(builder, ProviderButtonArgs{
		.id = u"ayu/translator/articleProvider"_q,
		.title = tr::ayu_TranslatorArticleProvider(),
		.forArticles = true,
		.getter = [] {
			return AyuSettings::getInstance().articleTranslationProvider();
		},
		.value = settings.articleTranslationProviderValue(),
		.setter = [](TranslationProvider provider) {
			AyuSettings::getInstance().setArticleTranslationProvider(
				provider);
		},
		.shown = settings.enableSeparateArticleTranslatorValue(),
	});
	builder.addSkip();
	builder.addDividerText(tr::ayu_TranslatorSeparateArticleNotice());
}

void BuildExperimental(SectionBuilder &builder, AyuSectionBuilder &ayu) {
	builder.addSubsectionTitle(tr::ayu_TranslatorExperimentalHeader());

	ayu.addSettingToggle({
		.id = u"ayu/translator/googleExp"_q,
		.title = tr::ayu_TranslatorGoogleExp(),
		.getter = &AyuSettings::googleTranslateExp,
		.setter = &AyuSettings::setGoogleTranslateExp,
	});
	ayu.addSettingToggle({
		.id = u"ayu/translator/keepPreferences"_q,
		.title = tr::ayu_TranslatorKeepPreferences(),
		.getter = &AyuSettings::keepTranslatorPreferences,
		.setter = &AyuSettings::setKeepTranslatorPreferences,
	});
	builder.addSkip();
	builder.addDividerText(tr::ayu_TranslatorKeepPreferencesNotice());
}

const auto kMeta = BuildHelper({
	.id = AyuTranslator::Id(),
	.parentId = AyuMain::Id(),
	.title = &tr::ayu_CategoryTranslator,
	.icon = &st::menuIconTranslate,
}, [](SectionBuilder &builder) {
	auto ayu = AyuSectionBuilder(builder);

	builder.addSkip();
	BuildOptions(builder, ayu);
	builder.addSkip();
	builder.addDivider();
	builder.addSkip();
	BuildTranslate(builder, ayu);
	builder.addSkip();
	builder.addDivider();
	builder.addSkip();
	BuildAiTranslator(builder, ayu);
	builder.addSkip();
	builder.addDivider();
	builder.addSkip();
	BuildArticle(builder, ayu);
	builder.addSkip();
	builder.addDivider();
	builder.addSkip();
	BuildExperimental(builder, ayu);
	builder.addSkip();
});

} // namespace

rpl::producer<QString> AyuTranslator::title() {
	return tr::ayu_CategoryTranslator();
}

AyuTranslator::AyuTranslator(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

void AyuTranslator::setupContent() {
	// Android parity: AI translator without an API key can't work, so when
	// leaving the section with it selected restore the previous provider.
	const auto initial = AyuSettings::getInstance().translationProvider();
	lifetime().add([=] {
		auto &settings = AyuSettings::getInstance();
		if ((settings.translationProvider() == TranslationProvider::LLM)
			&& !Llm::Config::IsAvailable()) {
			settings.setTranslationProvider(
				(initial == TranslationProvider::LLM)
					? TranslationProvider::Google
					: initial);
		}
	});
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	build(content, kMeta.build);
	Ui::ResizeFitChild(this, content);
}

Type AyuTranslatorId() {
	return AyuTranslator::Id();
}

} // namespace Settings
