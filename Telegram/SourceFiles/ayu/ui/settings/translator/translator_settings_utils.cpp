// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/ui/settings/translator/translator_settings_utils.h"

#include "lang_auto.h"
#include "ayu/ui/boxes/text_input_box.h"
#include "styles/style_settings.h"
#include "ui/layers/generic_box.h"
#include "window/window_session_controller.h"

namespace Settings::TranslatorUi {
namespace {

[[nodiscard]] QString ElideValue(const QString &value) {
	constexpr auto kMax = 28;
	const auto simplified = value.simplified();
	return (simplified.size() > kMax)
		? (simplified.left(kMax) + QChar(0x2026))
		: simplified;
}

} // namespace

Ui::SettingsButton *AddTextValueButton(
		Builder::SectionBuilder &builder,
		TextValueArgs &&args) {
	const auto controller = builder.controller();
	const auto secret = args.secret;

	const auto emptyLabel = args.emptyLabel;
	auto label = std::move(
		args.value
	) | rpl::map([=](const QString &value) {
		if (value.trimmed().isEmpty()) {
			return emptyLabel.isEmpty()
				? tr::ayu_TranslatorValueNone(tr::now)
				: emptyLabel;
		}
		return secret
			? tr::ayu_TranslatorValueSet(tr::now)
			: ElideValue(value);
	});

	const auto boxTitle = args.boxTitle;
	const auto placeholder = args.placeholder;
	const auto description = args.description;
	const auto helpUrl = args.helpUrl;
	const auto helpUrlGetter = args.helpUrlGetter;
	const auto multiline = args.multiline;
	const auto showReset = args.showReset;
	const auto getter = args.getter;
	const auto setter = args.setter;
	const auto validate = args.validate;

	return builder.addButton({
		.id = args.id,
		.title = std::move(args.title),
		.st = &st::settingsButtonNoIcon,
		.label = std::move(label),
		.onClick = [=] {
			if (!controller) {
				return;
			}
			const auto url = helpUrlGetter ? helpUrlGetter() : helpUrl;
			controller->show(Box([=](not_null<Ui::GenericBox*> box) {
				AyuUi::TextInputBox(box, AyuUi::TextInputBoxArgs{
					.title = rpl::single(boxTitle),
					.placeholder = rpl::single(placeholder),
					.description = description,
					.helpLinkText = url.isEmpty()
						? QString()
						: tr::ayu_TranslatorHowToObtain(tr::now),
					.helpLinkUrl = url,
					.value = getter ? getter() : QString(),
					.multiline = multiline,
					.showReset = showReset,
					.validate = validate,
					.save = setter,
				});
			}));
		},
		.keywords = std::move(args.keywords),
		.shown = std::move(args.shown),
	});
}

} // namespace Settings::TranslatorUi
