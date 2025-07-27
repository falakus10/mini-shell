#include "minishell.h"

char *ft_strcpy(char *dest, const char *src)
{
	int i = 0;

	while (src[i])
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return dest;
}

char *ft_strcat(char *dest, const char *src)
{
	int i = 0;
	int j = 0;

	// dest'in sonuna git
	while (dest[i])
		i++;

	// src'yi dest'in sonuna kopyala
	while (src[j])
	{
		dest[i] = src[j];
		i++;
		j++;
	}
	dest[i] = '\0';
	return dest;
}

int	ft_strcmp(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2))
	{
		s1++;
		s2++;
	}
	return ((unsigned char)*s1 - (unsigned char)*s2);
}

t_env	*add_new_node3(t_env **env_list)
{
	t_env	*node;
	t_env	*temp;
	node = malloc(sizeof(t_env));
	node->next = NULL;
	node->value = NULL;
	if (*env_list == NULL)
		*env_list = node;
	else
	{
		temp = *env_list;
		while (temp->next != NULL)
			temp = temp->next;
		temp->next = node;
	}
	return (node);
}

t_env   **take_env(char **env)
{
	int	i;
	int	j;
	t_env **env_list;
	t_env *current;

	i = 0;
	env_list = malloc(sizeof(t_env*));
	*env_list = NULL;
	while (env[i] != NULL)
	{
		j = 0;
		current = add_new_node3(env_list);
		current->line = ft_strdup(env[i]);
		while (env[i][j] != '\0')
		{
			if (env[i][j] == '=')
			{
				current->value = ft_strdup(&env[i][j + 1]);
				break;
			}
			j++;
		}
		i++;
	}
	return (env_list);
}
