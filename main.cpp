// main.cpp
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cmath>

#include "llama.h"
#include "whisper.h"
#include <thread>

const std::string ASR_MODEL_PATH = "models/ggml-tiny.en.bin";
const std::string LLM_MODEL_PATH = "models/phi-2.Q4_K_M.gguf";

bool loadAsrModel(whisper_context *&ctx);
bool loadLlmModel(llama_context *&ctx, llama_model *&model);
std::string llama_simple_generate(llama_model *model, llama_context *ctx, const std::string &prompt, int max_tokens);

int main()
{
	llama_backend_init();

	std::cout << "============================================" << std::endl;
	std::cout << "Inicializando o CEREBRO do George..." << std::endl;
	std::cout << "Whisper Versao: " << whisper_print_system_info() << std::endl;
	std::cout << "Llama Versao: " << llama_print_system_info() << "\n"
			  << std::endl;

	whisper_context *asr_ctx = nullptr;

	if (!loadAsrModel(asr_ctx))
	{
		return 1;
	}

	llama_context *llm_ctx = nullptr;
	llama_model *llm_model = nullptr;

	if (!loadLlmModel(llm_ctx, llm_model))
	{
		if (asr_ctx)
			whisper_free(asr_ctx);
		if (llm_ctx)
			llama_free(llm_ctx);
		if (llm_model)
			llama_free_model(llm_model);

		llama_backend_free();
		return 1;
	}

	const std::string human_prompt = "Hello! Tell me a fun fact about robots.";

	std::cout << "\n--- CICLO DE INTERACAO SIMULADO ---" << std::endl;
	std::cout << "Humano (Whisper Transcreveu): " << human_prompt << std::endl;

	std::string robot_response = llama_simple_generate(llm_model, llm_ctx, human_prompt, 128);

	std::cout << "\nRobo (LLM Responde): " << robot_response << std::endl;
	std::cout << "-----------------------------------\n"
			  << std::endl;

	// Libere todos os recursos
	if (asr_ctx)
		whisper_free(asr_ctx);
	if (llm_ctx)
		llama_free(llm_ctx);
	if (llm_model)
		llama_free_model(llm_model);
	llama_backend_free();

	std::cout << "Geoge desligado. Contextos liberados." << std::endl;
	std::cout << "============================================" << std::endl;

	return 0;
}

bool loadAsrModel(whisper_context *&ctx)
{
	std::cout << "Carregando ASR (Whisper): " << ASR_MODEL_PATH << std::endl;

	ctx = whisper_init_from_file(ASR_MODEL_PATH.c_str());

	if (ctx == nullptr)
	{
		std::cerr << "ERRO: Falha ao carregar o modelo ASR." << std::endl;
		return false;
	}

	std::cout << "ASR carregado com sucesso." << std::endl;
	return true;
}

bool loadLlmModel(llama_context *&ctx, llama_model *&model)
{
	std::cout << "Carregando LLM (Llama.cpp): " << LLM_MODEL_PATH << std::endl;

	// Configura��es do MODELO (Est�tico)
	llama_model_params model_params = llama_model_default_params();

	// --- PERFORMANCE 1: GPU Offloading ---
	// Joga o m�ximo de camadas para a VRAM da placa de v�deo.
	// Use 99 (ou -1 em algumas vers�es) para tentar carregar tudo.
	// Se a compila��o n�o tiver suporte a CUDA/Vulkan, isso � ignorado.
	model_params.n_gpu_layers = 99;

	// Configura��es do CONTEXTO (Din�mico)
	llama_context_params ctx_params = llama_context_default_params();

	// Aumentei de 512 para 2048, pois o Phi-2 aguenta e 512 � muito pouco para conversas
	ctx_params.n_ctx = 2048;

	// --- PERFORMANCE 2: Threads da CPU ---
	// Detecta o n�mero de threads l�gicas da sua CPU.
	// Dica: Se o computador ficar travando, troque por um n�mero fixo (ex: 4 ou 6).
	unsigned int threads = std::thread::hardware_concurrency();
	int n_threads = (threads > 0) ? threads : 4; // Fallback para 4 se falhar

	ctx_params.n_threads = n_threads;		// Threads para gera��o
	ctx_params.n_threads_batch = n_threads; // Threads para processar o prompt inicial

	model = llama_load_model_from_file(LLM_MODEL_PATH.c_str(), model_params);

	if (model == nullptr)
	{
		std::cerr << "ERRO: Falha ao carregar o modelo LLM GGUF. (Verifique o caminho: " << LLM_MODEL_PATH << ")" << std::endl;
		return false;
	}

	ctx = llama_new_context_with_model(model, ctx_params);

	if (ctx == nullptr)
	{
		std::cerr << "ERRO: Falha ao criar o contexto LLM." << std::endl;
		llama_free_model(model);
		return false;
	}

	std::cout << "LLM carregado com sucesso." << std::endl;
	std::cout << "Config de Performance: " << n_threads << " threads CPU | GPU Layers solicitados: " << model_params.n_gpu_layers << std::endl;

	return true;
}

std::string llama_simple_generate(
	llama_model *model,
	llama_context *ctx,
	const std::string &prompt,
	int max_tokens = 128)
{

	const llama_vocab *vocab = llama_model_get_vocab(model);
	int vocab_size = llama_n_vocab(vocab);

	// ---------------------------------------------------------
	// PREPARA��O DO PROMPT
	// ---------------------------------------------------------
	std::vector<llama_token> tokens_prompt(prompt.size() + 32);
	int n_tokens = llama_tokenize(
		vocab, prompt.c_str(), prompt.size(),
		tokens_prompt.data(), tokens_prompt.size(), true, false);
	if (n_tokens < 0)
		throw std::runtime_error("Tokenization failed");
	tokens_prompt.resize(n_tokens);

	// Aloca um batch REUTILIZ�VEL grande o suficiente para o prompt
	// Se o prompt for maior que 2048, aumente este valor ou trate em chunks
	llama_batch batch = llama_batch_init(std::max((int)tokens_prompt.size(), 1), 0, 1);

	// Configura o batch para o prompt
	for (size_t i = 0; i < tokens_prompt.size(); ++i)
	{
		batch.token[i] = tokens_prompt[i];
		batch.pos[i] = i;
		batch.n_seq_id[i] = 1;
		batch.seq_id[i][0] = 0;
		batch.logits[i] = (i == tokens_prompt.size() - 1); // S� calcula logits pro �ltimo
	}
	batch.n_tokens = tokens_prompt.size();

	if (llama_decode(ctx, batch) != 0)
	{
		llama_batch_free(batch);
		throw std::runtime_error("Prompt evaluation failed.");
	}

	// ---------------------------------------------------------
	// LOOP DE GERA��O (HOT PATH)
	// ---------------------------------------------------------
	int n_past = n_tokens;
	std::string response;

	// Alocamos mem�ria para string de resposta para evitar reallocs pequenos
	response.reserve(max_tokens * 4);

	// Pega logits do prompt
	float *logits = llama_get_logits_ith(ctx, batch.n_tokens - 1);

	// Otimiza��o: Busca direta no array (sem alocar vector<candidates>)
	llama_token next_token = 0;
	float max_val = -INFINITY;
	for (int t = 0; t < vocab_size; ++t)
	{
		if (logits[t] > max_val)
		{
			max_val = logits[t];
			next_token = t;
		}
	}

	for (int i = 0; i < max_tokens; ++i)
	{
		if (next_token == llama_token_eos(vocab))
			break;

		// Decodifica token para string
		char buf[16];
		int n_out = llama_token_to_piece(vocab, next_token, buf, sizeof(buf), 0, false);
		if (n_out > 0)
			response.append(buf, n_out);

		// Reutiliza o batch (sem malloc/free aqui dentro)
		batch.n_tokens = 1;
		batch.token[0] = next_token;
		batch.pos[0] = n_past++;
		batch.n_seq_id[0] = 1;
		batch.seq_id[0][0] = 0;
		batch.logits[0] = true;

		if (llama_decode(ctx, batch) != 0)
			break;

		// Pega novos logits
		logits = llama_get_logits_ith(ctx, 0);

		// Greedy Sampling otimizado
		max_val = -INFINITY;
		for (int t = 0; t < vocab_size; ++t)
		{
			if (logits[t] > max_val)
			{
				max_val = logits[t];
				next_token = t;
			}
		}
	}

	llama_batch_free(batch); // Libera apenas uma vez no final
	return response;
}