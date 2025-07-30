#include "minishell.h"

void	pass_cmd_blk(t_command_block **cmd, t_command_block **new,
		t_command_block **tmp)  //sonraki komut bloğuna geçiyor
{
	if ((*cmd) == NULL)
	{
		(*cmd) = (*new);
		(*tmp) = (*cmd);
	}
	else
	{
		(*tmp)->next = (*new);
		(*tmp) = (*tmp)->next;
	}
}

char *file_path(char *file)
{
	char buf[256];
	char *full_path;
	//file ismi $ ile başlıyorsa hata
	getcwd(buf, sizeof(buf));
	full_path = malloc(ft_strlen(buf) + ft_strlen(file) + 2); // / ve \0 için +2
	if(!full_path)
	{
		printf("allocation error\n");
		ft_error(); //ne return olmalı 
	}
	ft_strcpy(full_path,buf);
	ft_strcat(full_path,"/");
	ft_strcat(full_path,file);
	return (full_path);
}
//son commande neden null geldi, hata mesajlarını neden basmadı, heredoc tan gelen fd neden atanmadı ?
void assign_fd(t_command_block **tmp_blk, t_joined_lexer_list **tmp_list) // açılan fd'leri close yapmadım henüz, kullanılmayacak olanları close yap
{
	char *file_pth;
	int type;

	type = (*tmp_list)->type;
	file_pth = file_path((*tmp_list)->next->token);

	if(type == REDIR_IN) //dosya isminin $ ile başlama durumuna bakılabilir
	{
		if((*tmp_blk)->file_err == 0) //bu komut bloğunda henüz redirection hatası çıkmadıysa 
		{
			(*tmp_blk)->input_fd = open(file_pth, O_RDONLY);
			if ((*tmp_blk)->input_fd == -1)
			{
				if(errno == EISDIR)
					printf("bash: %s: Is a directory\n", (*tmp_list)->next->token);
				else if(errno == ENOENT)
					printf("bash: %s: No such file or directory\n", (*tmp_list)->next->token);
				else 
					perror("bash");
				(*tmp_blk)->file_err = 1; //bu komut bloğunda file_error var demek. //Dolayısıyla bu komut bloğundaki diğer redirectionlara bakmayacak
				//flag tutulacak //eğer öncesinde input_fd hatası varsa onu da tutmalıyım ki (flag olarak olabilir) hem daha sonrasında input_fd hatası olursa onları basmasın hem de komut hatası varsa onu kontrol etmek için //bu flag sayesinde ilgili komut bloğu çalıştırılmayacak
			}
		}
	}
	else if(type == REDIR_OUT)
	{
		if((*tmp_blk)->file_err == 0)
		{
			(*tmp_blk)->output_fd = open(file_pth, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if ((*tmp_blk)->input_fd == -1)
			{
				if(errno == EISDIR)
					printf("bash: %s: Is a directory\n", (*tmp_list)->next->token);
				else if(errno == ENOENT)
					printf("bash: %s: No such file or directory\n", (*tmp_list)->next->token);
				else
					perror("bash");
				(*tmp_blk)->file_err = 1;
			}
		}
	}
	else if(type == APPEND)
	{
		if((*tmp_blk)->file_err == 0)
		{
			(*tmp_blk)->output_fd = open(file_pth, O_WRONLY | O_CREAT | O_APPEND, 0644);
			if ((*tmp_blk)->output_fd == -1)//bu olmaz zaten silinebilir
			{
				if(errno == EISDIR)
					printf("bash: %s: Is a directory\n", (*tmp_list)->next->token);
				else if(errno == ENOENT)
					printf("bash: %s: No such file or directory\n", (*tmp_list)->next->token);
				else
					perror("bash");
				(*tmp_blk)->file_err = 1;
			}
		}
	}
}

void	handle_redirect_token(t_joined_lexer_list **temp,
		t_command_block **temp_block,t_mng_heredocs *mng_heredocs)
{
		if((*temp)->type == HEREDOC)
		{
			if(mng_heredocs->heredoc_flags[mng_heredocs->index]) //bu varsa redir_in ler hiç çalışmasın diyemem çünkü o zaman hata mesajını alamam (ama hata almayana kadarla sınırlasam ?)
				(*temp_block)->input_fd = mng_heredocs->heredoc_fds[mng_heredocs->index];
			*temp = (*temp)->next;
			return;
		}
		(*temp_block)->files = append_to_array((*temp_block)->files,
				(*temp_block)->operator_count, (*temp)->next->token);
		assign_fd((temp_block),(temp)); //!!! fd'ler güncelleniyor ama önceki fd'ler kapanmıyor !!!
		(*temp_block)->operator_count++; // işime yarıyor mu ? yarıyorsa işlenişi değişebilir.
		if(mng_heredocs->heredoc_flags[mng_heredocs->index]) //bu varsa redir_in ler hiç çalışmasın diyemem çünkü o zaman hata mesajını alamam (ama hata almayana kadarla sınırlasam ?)
			(*temp_block)->input_fd = mng_heredocs->heredoc_fds[mng_heredocs->index];
		*temp = (*temp)->next;
}

//ardından son komut neden null geliyor onu çöz

void	handle_token_logic(t_joined_lexer_list **tmp, t_command_block **tmp_blk,
		t_pipeline_utils *utils,t_mng_heredocs *mng_heredocs)
{
	int fd_count;

	fd_count = 0;
	if ((*tmp)->next != NULL && ((*tmp)->type == REDIR_IN 		//veya != WORD && != PIPE
			|| (*tmp)->type == REDIR_OUT || (*tmp)->type == APPEND
			|| (*tmp)->type == HEREDOC))
		handle_redirect_token(tmp, tmp_blk,mng_heredocs);
	else if (((*tmp)->type == WORD
			|| (*tmp)->type == S_QUOTE || (*tmp)->type == D_QUOTE))
	{
		if (!utils->is_cmd_pointed)
		{
			if (is_builtin((*tmp)->token)) //command'e burada atama yapılacak
				(*tmp_blk)->command = ft_strdup((*tmp)->token); //tokenları direk liste olarak free'leriz o yüzden *tmp->token olarak atamayalım
			else //command ataması create_path içinde oluyor
			{
				if(!create_path((*tmp_blk),(*tmp)->token) && (*tmp_blk)->cmd_err == 0)
				{
					(*tmp_blk)->command = (*tmp)->token;
					(*tmp_blk)->wrong_cmd = (*tmp)->token; //strdup ile mi atmalıyım (bence illa strdup a gerek yok parserdan sonra joined lexer list freelenebilir)!!!
					(*tmp_blk)->cmd_err = 1;
				}
			}
			(*tmp_blk)->args = append_to_array((*tmp_blk)->args,(*tmp_blk)->argument_count,(*tmp)->token);
			utils->is_cmd_pointed = 1;
			(*tmp_blk)->argument_count++;
		}
		else if (utils->is_cmd_pointed)
		{
			(*tmp_blk)->args = append_to_array((*tmp_blk)->args,
				(*tmp_blk)->argument_count, (*tmp)->token);
			(*tmp_blk)->argument_count++;
		}
	}
	fd_count++; //bunu niye tuttum ?
	(*tmp) = (*tmp)->next;
}
