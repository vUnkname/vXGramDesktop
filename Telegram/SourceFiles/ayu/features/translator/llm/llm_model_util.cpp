// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/llm/llm_model_util.h"

#include "ayu/features/translator/llm/llm_presets.h"

#include <optional>
#include <QtCore/QJsonArray>
#include <QtCore/QRegularExpression>
#include <QtCore/QUrl>

namespace Ayu::Translator::Llm::ModelUtil {
namespace {

[[nodiscard]] QString LowerBase(const QString &model) {
	return BaseModelName(model).toLower();
}

[[nodiscard]] QJsonObject Obj(const QString &key, const QJsonValue &value) {
	auto result = QJsonObject();
	result.insert(key, value);
	return result;
}

[[nodiscard]] std::optional<QString> RouterModelProvider(
		const QString &model) {
	const auto trimmed = model.trimmed();
	if (trimmed.isEmpty() || !trimmed.contains('/')) {
		return std::nullopt;
	}
	return trimmed.left(trimmed.indexOf('/')).toLower();
}

void PutProviderOptions(
		QJsonObject &request,
		const QString &provider,
		const QJsonObject &options) {
	auto providerOptions = request.value(u"providerOptions"_q).toObject();
	providerOptions.insert(provider, options);
	request.insert(u"providerOptions"_q, providerOptions);
}

void ApplyOriginal(QJsonObject &request, const QString &model) {
	if (IsOpenaiCompatibleReasoning(model) || IsGemma4(model)) {
		request.insert(u"reasoning_effort"_q, ReasoningEffort(model));
	} else if (IsDeepSeekV4(model)) {
		request.insert(u"thinking"_q, Obj(u"type"_q, u"disabled"_q));
	}
}

[[nodiscard]] bool ApplyRouter(
		QJsonObject &request,
		int preset,
		const QString &model) {
	const auto routerProvider = RouterModelProvider(model);
	if (!routerProvider) {
		return false;
	}
	const auto &provider = *routerProvider;
	if (preset == OpenRouter) {
		if (provider == u"google"_q) {
			request.insert(
				u"reasoning"_q,
				Obj(u"effort"_q, ReasoningEffort(model)));
			return true;
		} else if ((provider == u"openai"_q)
			&& model.contains(u"gpt-oss"_q)) {
			request.insert(
				u"reasoning"_q,
				Obj(u"effort"_q, u"minimal"_q));
			return true;
		}
		request.insert(u"reasoning"_q, Obj(u"effort"_q, u"none"_q));
		return true;
	} else if (preset == VercelAiGateway) {
		if (provider == u"google"_q) {
			const auto config = IsGemini3(model)
				? Obj(u"thinkingLevel"_q, u"minimal"_q)
				: Obj(u"thinkingBudget"_q, 0);
			PutProviderOptions(
				request,
				u"google"_q,
				Obj(u"thinkingConfig"_q, config));
			return true;
		} else if ((provider == u"deepseek"_q) && IsDeepSeekV4(model)) {
			PutProviderOptions(
				request,
				u"deepseek"_q,
				Obj(u"thinking"_q, Obj(u"type"_q, u"disabled"_q)));
			return true;
		}
		PutProviderOptions(
			request,
			provider,
			Obj(u"reasoning"_q, Obj(u"effort"_q, u"none"_q)));
		return true;
	}
	return false;
}

} // namespace

QString BaseModelName(const QString &model) {
	const auto trimmed = model.trimmed();
	if (trimmed.isEmpty()) {
		return {};
	}
	const auto slash = trimmed.lastIndexOf('/');
	return (slash >= 0) ? trimmed.mid(slash + 1) : trimmed;
}

bool IsGPT5(const QString &model) {
	const auto base = LowerBase(model);
	return !base.startsWith(u"gpt-5."_q)
		&& base.startsWith(u"gpt-5"_q)
		&& !base.contains(u"instant"_q)
		&& !base.contains(u"chat"_q);
}

bool IsGemma4(const QString &model) {
	const auto base = LowerBase(model);
	return base.contains(u"gemma4"_q) || base.contains(u"gemma-4"_q);
}

bool IsGemini3(const QString &model) {
	return LowerBase(model).startsWith(u"gemini-3"_q);
}

bool IsGeminiLegacy(const QString &model) {
	const auto base = LowerBase(model);
	return base.startsWith(u"gemini-2"_q)
		|| base.startsWith(u"gemini-3-"_q)
		|| base.startsWith(u"gemini-3.1"_q);
}

bool IsDeepSeekV4(const QString &model) {
	return LowerBase(model).startsWith(u"deepseek-v4"_q);
}

bool IsReasoning(const QString &model) {
	return IsOpenaiCompatibleReasoning(model)
		|| IsGemma4(model)
		|| IsDeepSeekV4(model);
}

bool IsOpenaiCompatibleReasoning(const QString &model) {
	const auto base = LowerBase(model);
	return (base.contains(u"gemini"_q) && base.contains(u"flash"_q))
		|| (base.startsWith(u"gpt-5"_q)
			&& !base.contains(u"instant"_q)
			&& !base.contains(u"chat"_q))
		|| base.startsWith(u"gpt-oss"_q)
		|| base.startsWith(u"grok-4.3"_q)
		|| base.startsWith(u"glm-5"_q)
		|| base.startsWith(u"hy3"_q)
		|| base.startsWith(u"inkling"_q)
		|| base.startsWith(u"kimi-k2.5"_q)
		|| base.startsWith(u"kimi-k2.6"_q)
		|| base.startsWith(u"kimi-k3"_q)
		|| base.startsWith(u"nemotron-3"_q)
		|| base.startsWith(u"qwen3"_q);
}

QString ReasoningEffort(const QString &model) {
	const auto base = LowerBase(model);
	if (base.startsWith(u"gpt-oss"_q)) {
		return u"low"_q;
	} else if (base.startsWith(u"gpt-5."_q)) {
		return u"none"_q;
	} else if (base.startsWith(u"gpt-5"_q)) {
		return u"minimal"_q;
	} else if (base.startsWith(u"gemini"_q)
		&& (base.endsWith(u"latest"_q) || !IsGeminiLegacy(model))) {
		return u"minimal"_q;
	} else if (IsGemma4(model)) {
		return u"minimal"_q;
	}
	return u"none"_q;
}

void ApplyReasoningParameters(
		QJsonObject &request,
		const QString &baseUrl,
		const QString &model) {
	if (!IsReasoning(model)) {
		return;
	}
	auto preset = std::optional<int>();
	if (baseUrl == PresetBaseUrl(Gemini)) {
		preset = Gemini;
	} else if (baseUrl == PresetBaseUrl(OpenRouter)) {
		preset = OpenRouter;
	} else if (baseUrl == PresetBaseUrl(VercelAiGateway)) {
		preset = VercelAiGateway;
	}
	if (IsGemma4(model) && (preset != Gemini)) {
		return;
	}
	if (preset && ApplyRouter(request, *preset, model)) {
		return;
	}
	ApplyOriginal(request, model);
}

bool SupportsTemperature(const QString &model) {
	const auto base = LowerBase(model);
	return !base.startsWith(u"gpt-5"_q)
		&& (!base.startsWith(u"gemini"_q) || IsGeminiLegacy(model));
}

QStringList StripModelsPrefix(const QStringList &models) {
	auto result = QStringList();
	for (const auto &model : models) {
		auto id = model.trimmed();
		if (id.startsWith(u"models/"_q)) {
			id = id.mid(7);
		}
		if (!id.isEmpty() && !result.contains(id)) {
			result.push_back(id);
		}
	}
	return result;
}

bool IsOpenRouterFreeModel(const QString &model) {
	return model.trimmed().endsWith(u":free"_q, Qt::CaseInsensitive);
}

QString SanitizeResponse(const QString &model, const QString &content) {
	auto result = content.trimmed();
	if (result.isEmpty()) {
		return {};
	}
	if (IsGemma4(model)) {
		static const auto regex = QRegularExpression(
			u"<thought>.*?</thought>"_q,
			QRegularExpression::CaseInsensitiveOption
				| QRegularExpression::DotMatchesEverythingOption);
		result = result.remove(regex).trimmed();
	}
	return result;
}

QString NormalizeBaseUrl(const QString &url) {
	auto result = url.trimmed();
	if (result.isEmpty()) {
		return {};
	}
	if (!result.startsWith(u"http://"_q, Qt::CaseInsensitive)
		&& !result.startsWith(u"https://"_q, Qt::CaseInsensitive)) {
		result = u"https://"_q + result;
	}
	const auto trimSlashes = [&] {
		while (result.endsWith('/')) {
			result.chop(1);
		}
	};
	trimSlashes();
	const auto suffix = u"/chat/completions"_q;
	if (result.endsWith(suffix, Qt::CaseInsensitive)) {
		result.chop(suffix.size());
		trimSlashes();
	}
	return result;
}

bool IsValidBaseUrl(const QString &url) {
	const auto normalized = NormalizeBaseUrl(url);
	if (normalized.isEmpty()) {
		return true;
	}
	const auto parsed = QUrl(normalized, QUrl::StrictMode);
	return parsed.isValid()
		&& !parsed.host().isEmpty()
		&& ((parsed.scheme() == u"http"_q) || (parsed.scheme() == u"https"_q));
}

QString StripReasoningMarkup(const QString &text) {
	auto result = QString();
	auto i = 0;
	while (i < text.size()) {
		const auto open = text.indexOf(u"<think>"_q, i, Qt::CaseInsensitive);
		if (open < 0) {
			result += text.mid(i);
			break;
		}
		result += text.mid(i, open - i);
		const auto close = text.indexOf(
			u"</think>"_q,
			open + 7,
			Qt::CaseInsensitive);
		if (close < 0) {
			break;
		}
		i = close + 8;
	}
	auto start = 0;
	while ((start < result.size()) && result[start].isSpace()) {
		++start;
	}
	return result.mid(start);
}

} // namespace Ayu::Translator::Llm::ModelUtil
