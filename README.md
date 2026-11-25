# Sunflowers

**Sunflowers** es un juego de lógica y deducción inspirado en un buscaminas inverso implementado en C para terminal. Sobre un tablero de 8×8 se muestran desde el inicio una serie de casillas con pistas numéricas que indican cuántos girasoles hay en sus ocho casillas vecinas. El objetivo del jugador consiste en deducir dónde se esconden las 18 flores introduciendo coordenadas para marcarlas (`o`) o descartar casillas vacías (`x`). La partida se gana al descubrir todos los girasoles antes de cometer 3 fallos. 

NOTA: La generación del tablero es simple y el contenido de las pistas no determina de forma única la solución, por lo que en algunos casos puede ser imposible resolver el tablero sin el factor aleatorio.

## Compilación y ejecución

```bash
gcc sunflowers.c -o sunflowers
./sunflowers
```
