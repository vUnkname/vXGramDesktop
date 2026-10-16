// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/ui/boxes/text_input_box.h"

#include "lang_auto.h"
#include "lang/lang_keys.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_widgets.h"
#include "ui/layers/generic_box.h"
#include "ui/text/text_utilities.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"

namespace AyuUi {

void TextInputBox(
		not_null<Ui::GenericBox*> box,
		TextInputBoxArgs &&args) {
	box->setTitle(std::move(args.title));

	if (!args.description.isEmpty()) {
		auto text = TextWithEntities{ .text = args.description };
		if (!args.helpLinkText.isEmpty() && !args.helpLinkUrl.isEmpty()) {
			text.append('\n');
			text.append(Ui::Text::Link(args.helpLinkText, args.helpLinkUrl));
		}
		box->addRow(object_ptr<Ui::FlatLabel>(
			box,
			rpl::single(std::move(text)),
			st::boxLabel));
	}

	const auto field = box->addRow(object_ptr<Ui::InputField>(
		box,
		st::defaultInputField,
		args.multiline
			? Ui::InputField::Mode::MultiLine
			: Ui::InputField::Mode::NoNewlines,
		std::move(args.placeholder),
		args.value));
	if (args.multiline) {
		field->setMaxHeight(st::boxWideWidth / 2);
	}

	const auto allowEmpty = args.allowEmpty;
	const auto validate = std::move(args.validate);
	const auto saveCallback = std::move(args.save);
	const auto submit = [=] {
		const auto value = field->getLastText().trimmed();
		if ((!allowEmpty && value.isEmpty())
			|| (validate && !validate(value))) {
			field->showError();
			return;
		}
		if (saveCallback) {
			saveCallback(value);
		}
		box->closeBox();
	};

	if (args.showReset) {
		box->addLeftButton(tr::ayu_BoxActionReset(), [=] {
			field->setText(QString());
		});
	}
	box->addButton(tr::lng_settings_save(), submit);
	box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });

	if (!args.multiline) {
		field->submits(
		) | rpl::on_next(submit, field->lifetime());
	}
	box->setFocusCallback([=] {
		field->setFocusFast();
	});
}

} // namespace AyuUi
