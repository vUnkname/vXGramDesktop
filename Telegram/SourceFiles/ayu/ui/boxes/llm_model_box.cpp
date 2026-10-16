// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/ui/boxes/llm_model_box.h"

#include "lang_auto.h"
#include "ayu/ayu_settings.h"
#include "lang/lang_keys.h"
#include "ayu/features/translator/llm/llm_client.h"
#include "ayu/features/translator/llm/llm_config.h"
#include "ayu/features/translator/llm/llm_model_util.h"
#include "ayu/features/translator/llm/llm_presets.h"
#include "settings/settings_common.h"
#include "styles/style_boxes.h"
#include "styles/style_layers.h"
#include "styles/style_settings.h"
#include "ui/boxes/confirm_box.h"
#include "ui/layers/generic_box.h"
#include "ui/ui_utility.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"

#include <algorithm>
#include <QtCore/QPointer>

namespace AyuUi {
namespace {

using namespace Ayu::Translator;

constexpr auto kMaxShownModels = 300;

struct State {
	QPointer<Ui::GenericBox> box;
	Ui::InputField *field = nullptr;
	Ui::VerticalLayout *list = nullptr;
	QStringList models;
	QString error;
	bool loading = false;
	bool userEdited = false;
	bool suppress = false;
	bool pinDefault = false;
	QString defaultModel;
	QString baseUrl;
	QString apiKey;
	int preset = 0;
	Fn<void()> rebuild;
	Fn<void()> load;
};

void SortModels(int preset, QStringList &models) {
	std::sort(models.begin(), models.end(), [=](
			const QString &a,
			const QString &b) {
		if (preset == Llm::OpenRouter) {
			const auto aFree = Llm::ModelUtil::IsOpenRouterFreeModel(a);
			const auto bFree = Llm::ModelUtil::IsOpenRouterFreeModel(b);
			if (aFree != bFree) {
				return aFree;
			}
		}
		return a.compare(b, Qt::CaseInsensitive) < 0;
	});
}

} // namespace

void LlmModelBox(not_null<Ui::GenericBox*> box, int preset) {
	box->setTitle(tr::ayu_LlmModelName());

	const auto state = box->lifetime().make_state<State>();
	state->box = box.get();
	state->preset = preset;
	state->baseUrl = Llm::Config::EffectiveBaseUrl(preset);
	state->apiKey = Llm::Config::FirstApiKey(preset);
	state->defaultModel = Llm::PresetDefaultModel(preset);
	if (preset == Llm::Custom) {
		state->pinDefault = !AyuSettings::getInstance().llmApiUrl()
				.trimmed().isEmpty()
			? false
			: !state->defaultModel.isEmpty();
	} else {
		state->pinDefault = !state->defaultModel.isEmpty();
	}

	state->field = box->addRow(object_ptr<Ui::InputField>(
		box,
		st::defaultInputField,
		Ui::InputField::Mode::NoNewlines,
		tr::ayu_LlmModelName(),
		Llm::Config::EffectiveModel(preset)));
	state->list = box->addRow(
		object_ptr<Ui::VerticalLayout>(box),
		style::margins());

	const auto select = [=](const QString &model) {
		state->suppress = true;
		state->field->setText(model);
		state->suppress = false;
		state->userEdited = false;
		Ui::PostponeCall(state->box, [=] {
			state->rebuild();
		});
	};
	const auto addModelButton = [=](
			const QString &model,
			const QString &label = QString()) {
		const auto button = label.isEmpty()
			? not_null<Ui::SettingsButton*>(state->list->add(
				object_ptr<Ui::SettingsButton>(
					state->list,
					rpl::single(model),
					st::settingsButtonNoIcon)))
			: Settings::AddButtonWithLabel(
				state->list,
				rpl::single(model),
				rpl::single(label),
				st::settingsButtonNoIcon);
		button->setClickedCallback([=] {
			select(model);
		});
	};

	state->rebuild = [=] {
		state->list->clear();
		const auto query = state->userEdited
			? state->field->getLastText().trimmed().toLower()
			: QString();
		if (state->pinDefault) {
			addModelButton(
				state->defaultModel,
				tr::ayu_LlmDefault(tr::now));
		}
		if (state->loading) {
			state->list->add(
				object_ptr<Ui::FlatLabel>(
					state->list,
					tr::ayu_LlmLoading(),
					st::boxDividerLabel),
				st::boxRowPadding);
			return;
		}
		if (!state->error.isEmpty()) {
			if (!state->apiKey.isEmpty()) {
				const auto retry = state->list->add(
					object_ptr<Ui::SettingsButton>(
						state->list,
						tr::ayu_LlmModelsLoadFailed(),
						st::settingsButtonNoIcon));
				retry->setClickedCallback([=] {
					Ui::PostponeCall(state->box, [=] {
						state->load();
					});
				});
			}
			state->list->add(
				object_ptr<Ui::FlatLabel>(
					state->list,
					rpl::single(state->error),
					st::boxDividerLabel),
				st::boxRowPadding);
			return;
		}
		const auto parts = query.split(' ', Qt::SkipEmptyParts);
		auto shown = 0;
		for (const auto &model : state->models) {
			if (model.isEmpty()
				|| (state->pinDefault && (model == state->defaultModel))) {
				continue;
			}
			const auto lower = model.toLower();
			auto ok = true;
			for (const auto &part : parts) {
				if (!lower.contains(part)) {
					ok = false;
					break;
				}
			}
			if (!ok) {
				continue;
			}
			addModelButton(model);
			if (++shown >= kMaxShownModels) {
				break;
			}
		}
	};

	state->load = [=] {
		if (state->loading) {
			return;
		}
		if (state->apiKey.isEmpty()) {
			state->models.clear();
			state->error = tr::ayu_LlmApiKeyNotSet(tr::now);
			state->rebuild();
			return;
		}
		state->loading = true;
		state->error = QString();
		state->models.clear();
		state->rebuild();
		Llm::Client::instance().fetchModels(
			state->baseUrl,
			state->apiKey,
			[=](Llm::Response<QStringList> response) {
				if (!state->box) {
					return;
				}
				state->loading = false;
				if (response.ok()) {
					state->models = std::move(response.data);
					SortModels(state->preset, state->models);
				} else {
					state->error = response.error;
				}
				state->rebuild();
			});
	};

	state->field->changes(
	) | rpl::on_next([=] {
		if (state->suppress) {
			return;
		}
		state->userEdited = true;
		state->rebuild();
	}, state->field->lifetime());

	box->addLeftButton(tr::ayu_LlmTest(), [=] {
		const auto typed = state->field->getLastText().trimmed();
		const auto model = typed.isEmpty()
			? Llm::Config::EffectiveModel(state->preset)
			: typed;
		Llm::Client::instance().testChatCompletions(
			state->baseUrl,
			state->apiKey,
			model,
			[=](Llm::Response<QString> response) {
				if (!state->box) {
					return;
				}
				auto text = QString();
				if (response.ok()) {
					text = tr::ayu_LlmTestResultHttpStatusCode(tr::now)
						+ u": "_q + QString::number(response.httpCode)
						+ u"\n"_q + tr::ayu_LlmTestResultDuration(tr::now)
						+ u": "_q + QString::number(response.durationMs)
						+ u" ms\n"_q + tr::ayu_LlmTestResultResponse(tr::now)
						+ u": "_q + response.data;
				} else {
					text = response.error;
				}
				state->box->uiShow()->showBox(Ui::MakeInformBox(text));
			});
	});

	box->addButton(tr::lng_settings_save(), [=] {
		const auto value = state->field->getLastText().trimmed();
		Llm::Config::SetSavedModel(state->preset, value);
		box->closeBox();
	});
	box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });

	box->setFocusCallback([=] {
		state->field->setFocusFast();
	});

	state->rebuild();
	state->load();
}

} // namespace AyuUi
