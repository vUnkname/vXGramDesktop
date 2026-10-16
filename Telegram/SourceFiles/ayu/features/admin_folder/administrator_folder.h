// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

namespace Main {
class Session;
} // namespace Main

namespace Window {
class SessionController;
} // namespace Window

namespace AyuFeatures::AdminFolder {

// Whether the "Administrator" folder of this account still exists.
[[nodiscard]] bool Exists(not_null<Main::Session*> session);
[[nodiscard]] rpl::producer<bool> ExistsValue(
	not_null<Main::Session*> session);

// Collects every channel / group (main list + archive) where the current
// user is the creator or an admin and puts them into the "Administrator"
// folder. Creates the folder if it does not exist yet, otherwise refreshes
// its content. Shows limit boxes / toasts by itself.
void Apply(not_null<Window::SessionController*> controller);

} // namespace AyuFeatures::AdminFolder
