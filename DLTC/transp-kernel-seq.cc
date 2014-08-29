#include "transp-kernel.h"

void transpose_ij(const double * in, double * out, int di, int dj) {
  int i, ti, j, tj;
  for (ti = 0; ti <= di - 32; ti+=32) {
    for (tj = 0; tj <= dj - 32; tj+=32) {
      for (i = ti; i < ti+32; i++) {
        for (j = tj; j < tj+32; j++) {
          out[i*dj+j] = in[i*dj+j];
        }
      }
    }
    for (j = dj-(dj % 32); j < dj; j++) {
      for (i = ti; i < ti+32; i++) {
        out[i*dj+j] = in[i*dj+j];
      }
    }
  }
  for (i = di-(di % 32); i < di; i++) {
    for (j = 0; j < dj; j++) {
      out[i*dj+j] = in[i*dj+j];
    }
  }
}
void transpose_ji(const double * in, double * out, int di, int dj) {
  int i, ti, j, tj;
  for (ti = 0; ti <= di - 32; ti+=32) {
    for (tj = 0; tj <= dj - 32; tj+=32) {
      for (i = ti; i < ti+32; i++) {
        for (j = tj; j < tj+32; j++) {
          out[j*di+i] = in[i*dj+j];
        }
      }
    }
    for (j = dj-(dj % 32); j < dj; j++) {
      for (i = ti; i < ti+32; i++) {
        out[j*di+i] = in[i*dj+j];
      }
    }
  }
  for (i = di-(di % 32); i < di; i++) {
    for (j = 0; j < dj; j++) {
      out[j*di+i] = in[i*dj+j];
    }
  }
}
void transpose_ijkl(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[i*dj*dk*dl+j*dk*dl+k*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[i*dj*dk*dl+j*dk*dl+k*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[i*dj*dk*dl+j*dk*dl+k*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[i*dj*dk*dl+j*dk*dl+k*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[i*dj*dk*dl+j*dk*dl+k*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_ijlk(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[i*dj*dl*dk+j*dl*dk+l*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[i*dj*dl*dk+j*dl*dk+l*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[i*dj*dl*dk+j*dl*dk+l*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[i*dj*dl*dk+j*dl*dk+l*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[i*dj*dl*dk+j*dl*dk+l*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_ikjl(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[i*dk*dj*dl+k*dj*dl+j*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[i*dk*dj*dl+k*dj*dl+j*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[i*dk*dj*dl+k*dj*dl+j*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[i*dk*dj*dl+k*dj*dl+j*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[i*dk*dj*dl+k*dj*dl+j*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_iklj(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[i*dk*dl*dj+k*dl*dj+l*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[i*dk*dl*dj+k*dl*dj+l*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[i*dk*dl*dj+k*dl*dj+l*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[i*dk*dl*dj+k*dl*dj+l*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[i*dk*dl*dj+k*dl*dj+l*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_iljk(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[i*dl*dj*dk+l*dj*dk+j*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[i*dl*dj*dk+l*dj*dk+j*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[i*dl*dj*dk+l*dj*dk+j*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[i*dl*dj*dk+l*dj*dk+j*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[i*dl*dj*dk+l*dj*dk+j*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_ilkj(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[i*dl*dk*dj+l*dk*dj+k*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[i*dl*dk*dj+l*dk*dj+k*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[i*dl*dk*dj+l*dk*dj+k*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[i*dl*dk*dj+l*dk*dj+k*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[i*dl*dk*dj+l*dk*dj+k*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_jikl(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[j*di*dk*dl+i*dk*dl+k*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[j*di*dk*dl+i*dk*dl+k*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[j*di*dk*dl+i*dk*dl+k*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[j*di*dk*dl+i*dk*dl+k*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[j*di*dk*dl+i*dk*dl+k*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_jilk(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[j*di*dl*dk+i*dl*dk+l*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[j*di*dl*dk+i*dl*dk+l*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[j*di*dl*dk+i*dl*dk+l*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[j*di*dl*dk+i*dl*dk+l*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[j*di*dl*dk+i*dl*dk+l*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_jkil(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[j*dk*di*dl+k*di*dl+i*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[j*dk*di*dl+k*di*dl+i*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[j*dk*di*dl+k*di*dl+i*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[j*dk*di*dl+k*di*dl+i*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[j*dk*di*dl+k*di*dl+i*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_jkli(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[j*dk*dl*di+k*dl*di+l*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[j*dk*dl*di+k*dl*di+l*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[j*dk*dl*di+k*dl*di+l*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[j*dk*dl*di+k*dl*di+l*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[j*dk*dl*di+k*dl*di+l*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_jlik(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[j*dl*di*dk+l*di*dk+i*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[j*dl*di*dk+l*di*dk+i*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[j*dl*di*dk+l*di*dk+i*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[j*dl*di*dk+l*di*dk+i*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[j*dl*di*dk+l*di*dk+i*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_jlki(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[j*dl*dk*di+l*dk*di+k*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[j*dl*dk*di+l*dk*di+k*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[j*dl*dk*di+l*dk*di+k*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[j*dl*dk*di+l*dk*di+k*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[j*dl*dk*di+l*dk*di+k*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_kijl(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[k*di*dj*dl+i*dj*dl+j*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[k*di*dj*dl+i*dj*dl+j*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[k*di*dj*dl+i*dj*dl+j*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[k*di*dj*dl+i*dj*dl+j*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[k*di*dj*dl+i*dj*dl+j*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_kilj(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[k*di*dl*dj+i*dl*dj+l*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[k*di*dl*dj+i*dl*dj+l*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[k*di*dl*dj+i*dl*dj+l*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[k*di*dl*dj+i*dl*dj+l*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[k*di*dl*dj+i*dl*dj+l*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_kjil(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[k*dj*di*dl+j*di*dl+i*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[k*dj*di*dl+j*di*dl+i*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[k*dj*di*dl+j*di*dl+i*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[k*dj*di*dl+j*di*dl+i*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[k*dj*di*dl+j*di*dl+i*dl+l] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_kjli(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[k*dj*dl*di+j*dl*di+l*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[k*dj*dl*di+j*dl*di+l*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[k*dj*dl*di+j*dl*di+l*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[k*dj*dl*di+j*dl*di+l*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[k*dj*dl*di+j*dl*di+l*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_klij(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[k*dl*di*dj+l*di*dj+i*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[k*dl*di*dj+l*di*dj+i*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[k*dl*di*dj+l*di*dj+i*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[k*dl*di*dj+l*di*dj+i*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[k*dl*di*dj+l*di*dj+i*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_klji(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[k*dl*dj*di+l*dj*di+j*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[k*dl*dj*di+l*dj*di+j*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[k*dl*dj*di+l*dj*di+j*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[k*dl*dj*di+l*dj*di+j*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[k*dl*dj*di+l*dj*di+j*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_lijk(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[l*di*dj*dk+i*dj*dk+j*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[l*di*dj*dk+i*dj*dk+j*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[l*di*dj*dk+i*dj*dk+j*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[l*di*dj*dk+i*dj*dk+j*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[l*di*dj*dk+i*dj*dk+j*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_likj(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[l*di*dk*dj+i*dk*dj+k*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[l*di*dk*dj+i*dk*dj+k*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[l*di*dk*dj+i*dk*dj+k*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[l*di*dk*dj+i*dk*dj+k*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[l*di*dk*dj+i*dk*dj+k*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_ljik(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[l*dj*di*dk+j*di*dk+i*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[l*dj*di*dk+j*di*dk+i*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[l*dj*di*dk+j*di*dk+i*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[l*dj*di*dk+j*di*dk+i*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[l*dj*di*dk+j*di*dk+i*dk+k] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_ljki(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[l*dj*dk*di+j*dk*di+k*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[l*dj*dk*di+j*dk*di+k*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[l*dj*dk*di+j*dk*di+k*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[l*dj*dk*di+j*dk*di+k*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[l*dj*dk*di+j*dk*di+k*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_lkij(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[l*dk*di*dj+k*di*dj+i*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[l*dk*di*dj+k*di*dj+i*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[l*dk*di*dj+k*di*dj+i*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[l*dk*di*dj+k*di*dj+i*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[l*dk*di*dj+k*di*dj+i*dj+j] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}
void transpose_lkji(const double * in, double * out, int di, int dj, int dk, int dl) {
  int i, ti, j, tj, k, tk, l, tl;
  for (ti = 0; ti <= di - 4; ti+=4) {
    for (tj = 0; tj <= dj - 4; tj+=4) {
      for (tk = 0; tk <= dk - 4; tk+=4) {
        for (tl = 0; tl <= dl - 4; tl+=4) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                for (l = tl; l < tl+4; l++) {
                  out[l*dk*dj*di+k*dj*di+j*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
                }
              }
            }
          }
        }
        for (l = dl-(dl % 4); l < dl; l++) {
          for (i = ti; i < ti+4; i++) {
            for (j = tj; j < tj+4; j++) {
              for (k = tk; k < tk+4; k++) {
                out[l*dk*dj*di+k*dj*di+j*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
              }
            }
          }
        }
      }
      for (k = dk-(dk % 4); k < dk; k++) {
        for (i = ti; i < ti+4; i++) {
          for (j = tj; j < tj+4; j++) {
            for (l = 0; l < dl; l++) {
              out[l*dk*dj*di+k*dj*di+j*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
            }
          }
        }
      }
    }
    for (j = dj-(dj % 4); j < dj; j++) {
      for (i = ti; i < ti+4; i++) {
        for (k = 0; k < dk; k++) {
          for (l = 0; l < dl; l++) {
            out[l*dk*dj*di+k*dj*di+j*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
          }
        }
      }
    }
  }
  for (i = di-(di % 4); i < di; i++) {
    for (j = 0; j < dj; j++) {
      for (k = 0; k < dk; k++) {
        for (l = 0; l < dl; l++) {
          out[l*dk*dj*di+k*dj*di+j*di+i] = in[i*dj*dk*dl+j*dk*dl+k*dl+l];
        }
      }
    }
  }
}

