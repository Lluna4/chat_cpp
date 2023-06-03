#include <iostream>
#include <random>
#include <time.h>


static int	ft_intlen(int n)
{
	int	ret;

	ret = 1;
	while (n >= 10)
	{
		ret++;
		n = n / 10;
	}
	return (ret);
}

static char* ft_make_ret(int n, int sign)
{
	int		len;
	char* ret;

	len = ft_intlen(n) + sign;
	ret = (char *)calloc(len + 1, sizeof(char));
	if (!ret)
		return (0);
	len--;
	while (len >= 0)
	{
		ret[len] = (n % 10) + '0';
		n = n / 10;
		len--;
	}
	if (sign == 1)
		ret[0] = '-';
	return (ret);
}

char* ft_itoa(int n)
{
	char* ret;
	int		sign;

	sign = 0;
	if (n == -2147483648)
	{
		ret = (char *)malloc(12 * sizeof(char));
		if (!ret)
			return (0);
		memcpy(ret, "-2147483648", 12);
		return (ret);
	}
	if (n < 0)
	{
		n *= -1;
		sign = 1;
	}
	return (ft_make_ret(n, sign));
}

char* ft_strjoin(char const* s1, char const* s2)

{
	char* ret;
	int		n;

	n = -1;
	if (*s1 == '\0' && *s2 == '\0')
		return (_strdup(""));
	ret = (char *)calloc(strlen(s1) + strlen(s2) + 1, sizeof(char));
	if (!ret)
		return (0);
	while (*s1 != '\0')
	{
		n++;
		ret[n] = *s1;
		s1++;
	}
	while (*s2 != '\0')
	{
		n++;
		ret[n] = *s2;
		s2++;
	}
	return (ret);
}

char* generate_key()
{
	srand(time(NULL));
	int num;
	char* a = (char*)calloc(4, sizeof(char));
	while (strlen(a) < 1024)
	{
		num = rand();
		a = ft_strjoin(ft_itoa(num), a);
		srand(num);
	}
	return (a);
}

char *encode_text(char* text, const char *a)
{
	int index = 0;
	char* encoded_text = (char*)calloc(strlen(text) + 1, sizeof(char*));
	while (text[index])
	{
		encoded_text[index] = text[index] + a[index];
		index++;
	}
	return (encoded_text);
}

char* decode_text(char* encoded_text, const char* a)
{
	int index = 0;
	char* decoded_text = (char*)calloc(strlen(encoded_text) + 1, sizeof(char*));
	while (encoded_text[index])
	{
		decoded_text[index] = encoded_text[index] - a[index];
		index++;
	}
	return (decoded_text);
}

std::string decode_text(std::string encoded_text, char* key)
{
	int index = 0;
	std::string decoded_text;
	while (encoded_text[index])
	{
		decoded_text[index] = encoded_text[index] - key[index];
		index++;
	}
	return (decoded_text);
}