// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/ui/settings/settings_folder.h"

#include "lang_auto.h"
#include "ayu/features/admin_folder/administrator_folder.h"
#include "ayu/ui/settings/folder/administrator_preset_row.h"
#include "ayu/ui/settings/settings_main.h"
#include "main/main_session.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "styles/style_menu_icons.h"
#include "ui/ui_utility.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

namespace Settings {

using namespace Builder;

namespace {

void BuildAdministratorPreset(SectionBuilder &builder) {
	builder.add([](const WidgetContext &ctx) -> SectionBuilder::WidgetToAdd {
		const auto controller = ctx.controller;
		const auto session = &controller->session();

		auto row = object_ptr<AdministratorPresetRow>(ctx.container);
		const auto raw = row.data();

		AyuFeatures::AdminFolder::ExistsValue(
			session
		) | rpl::on_next([=](bool exists) {
			raw->setCreated(exists);
		}, raw->lifetime());

		rpl::merge(
			raw->addRequests(),
			raw->refreshRequests()
		) | rpl::on_next([=] {
			AyuFeatures::AdminFolder::Apply(controller);
		}, raw->lifetime());

		return { .widget = std::move(row) };
	});
}

const auto kMeta = BuildHelper({
	.id = AyuFolder::Id(),
	.parentId = AyuMain::Id(),
	.title = &tr::ayu_CategoryFolder,
	.icon = &st::menuIconAddToFolder,
}, [](SectionBuilder &builder) {
	builder.addSkip();
	builder.addSubsectionTitle(tr::ayu_FolderPresetsHeader());
	BuildAdministratorPreset(builder);
	builder.addSkip();
});

} // namespace

rpl::producer<QString> AyuFolder::title() {
	return tr::ayu_CategoryFolder();
}

AyuFolder::AyuFolder(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

void AyuFolder::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	build(content, kMeta.build);
	Ui::ResizeFitChild(this, content);
}

Type AyuFolderId() {
	return AyuFolder::Id();
}

} // namespace Settings
