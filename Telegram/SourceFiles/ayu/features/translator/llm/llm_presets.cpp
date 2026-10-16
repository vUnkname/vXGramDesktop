// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/llm/llm_presets.h"

#include "lang_auto.h"

namespace Ayu::Translator::Llm {

QString PresetName(int preset) {
	switch (preset) {
	case OpenAI: return u"OpenAI"_q;
	case Gemini: return u"Google AI Studio"_q;
	case Groq: return u"Groq"_q;
	case DeepSeek: return u"DeepSeek"_q;
	case XAI: return u"xAI"_q;
	case Cerebras: return u"Cerebras"_q;
	case OllamaCloud: return u"Ollama"_q;
	case OpenRouter: return u"OpenRouter"_q;
	case VercelAiGateway: return u"Vercel AI Gateway"_q;
	}
	return tr::ayu_LlmProviderCustom(tr::now);
}

QString PresetBaseUrl(int preset) {
	switch (preset) {
	case OpenAI: return u"https://api.openai.com/v1"_q;
	case Gemini: return u"https://generativelanguage.googleapis.com/v1beta/openai"_q;
	case Groq: return u"https://api.groq.com/openai/v1"_q;
	case DeepSeek: return u"https://api.deepseek.com/v1"_q;
	case XAI: return u"https://api.x.ai/v1"_q;
	case Cerebras: return u"https://api.cerebras.ai/v1"_q;
	case OllamaCloud: return u"https://ollama.com/v1"_q;
	case OpenRouter: return u"https://openrouter.ai/api/v1"_q;
	case VercelAiGateway: return u"https://ai-gateway.vercel.sh/v1"_q;
	}
	return {};
}

QString PresetDefaultModel(int preset) {
	switch (preset) {
	case OpenAI: return u"gpt-5.6-luna"_q;
	case Gemini: return u"gemini-flash-lite-latest"_q;
	case Groq: return u"openai/gpt-oss-120b"_q;
	case DeepSeek: return u"deepseek-v4-flash"_q;
	case XAI: return {};
	case Cerebras: return u"gemma-4-31b"_q;
	case OllamaCloud: return u"gemma4:31b"_q;
	case OpenRouter: return u"openrouter/free"_q;
	case VercelAiGateway: return u"google/gemma-4-31b-it"_q;
	}
	return u"gemini-flash-latest"_q;
}

QString PresetKeyUrl(int preset) {
	switch (preset) {
	case OpenAI: return u"https://platform.openai.com/api-keys"_q;
	case Gemini: return u"https://aistudio.google.com/app/apikey"_q;
	case Groq: return u"https://console.groq.com/keys"_q;
	case DeepSeek: return u"https://platform.deepseek.com/api_keys"_q;
	case XAI: return u"https://console.x.ai"_q;
	case Cerebras: return u"https://cloud.cerebras.ai"_q;
	case OllamaCloud: return u"https://ollama.com/settings/keys"_q;
	case OpenRouter: return u"https://openrouter.ai/keys"_q;
	case VercelAiGateway: return u"https://vercel.com/ai-gateway"_q;
	}
	return {};
}

} // namespace Ayu::Translator::Llm
