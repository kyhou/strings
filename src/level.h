#ifndef level// guardas de cabeçalho, impedem inclusões cíclicas
#define level

typedef struct EnvItem
{
  Rectangle rect;
  int blocking;
  Color color;
} EnvItem;

#endif