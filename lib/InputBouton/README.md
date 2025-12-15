# InputBouton Library

Librairie Arduino/PlatformIO pour gérer les boutons avec détection d'appuis courts, longs, et clics multiples.

## Caractéristiques

- ✅ Compatible avec tous types de boutons (tactiles TTP223, mécaniques, etc.)
- ✅ Détection d'appui court (simple clic)
- ✅ Détection d'appui long
- ✅ Détection de clics multiples (double, triple, quadruple, etc.)
- ✅ Configuration flexible des seuils de temps
- ✅ Nombre de clics maximum paramétrable
- ✅ Gestion automatique des états (HIGH/LOW)
- ✅ Pas de délai bloquant (non-blocking)

## Installation

### PlatformIO

1. Créer le dossier de la librairie :

```
lib/
└── InputBouton/
    ├── InputBouton.h
    ├── InputBouton.cpp
    ├── README.md
    └── examples/
        ├── BasicUsage/
        │   └── BasicUsage.ino
        └── AdvancedUsage/
            └── AdvancedUsage.ino
```

2. La librairie sera automatiquement détectée par PlatformIO.

## Utilisation

### Exemple basique

```cpp
#include <InputBouton.h>

#define BUTTON_PIN D5

// Pour un bouton tactile TTP223 (LOW par défaut, HIGH quand pressé)
InputBouton button(BUTTON_PIN, LOW);

// Pour un bouton mécanique avec INPUT_PULLUP (HIGH par défaut, LOW quand pressé)
// InputBouton button(BUTTON_PIN, HIGH);

void setup() {
  Serial.begin(9600);
  button.begin();

  // Configuration optionnelle
  button.setMaxClickCount(2);  // Détection jusqu'au double-clic
}

void loop() {
  ButtonEvent event = button.update();

  switch (event) {
    case BUTTON_SHORT_CLICK:
      Serial.println("Simple clic!");
      break;

    case BUTTON_MULTI_CLICK:
      Serial.print("Multi-clic: ");
      Serial.println(button.getClickCount());
      break;

    case BUTTON_LONG_PRESS:
      Serial.println("Appui long!");
      break;
  }
}
```

## API Reference

### Constructeur

```cpp
InputBouton(uint8_t buttonPin, boolean defaultButtonState = LOW)
```

- `buttonPin` : Broche GPIO connectée au bouton
- `defaultButtonState` : État du bouton au repos (LOW ou HIGH)
  - `LOW` pour boutons tactiles TTP223
  - `HIGH` pour boutons mécaniques avec pull-up interne

### Méthodes

#### Initialisation

```cpp
void begin()
```

Initialise la broche GPIO. À appeler dans `setup()`.

#### Configuration

```cpp
void setShortPressMax(unsigned long ms)
```

Définit la durée maximale d'un appui court (défaut : 500ms).

```cpp
void setLongPressMin(unsigned long ms)
```

Définit la durée minimale d'un appui long (défaut : 1000ms).

```cpp
void setMultiClickTimeout(unsigned long ms)
```

Définit le délai maximum entre deux clics pour être considérés comme multiples (défaut : 300ms).

```cpp
void setMaxClickCount(uint8_t maxClics)
```

Définit le nombre maximum de clics à détecter (défaut : 2 pour double-clic).

- `1` : uniquement simple clic
- `2` : jusqu'au double-clic
- `3` : jusqu'au triple-clic
- `4` : jusqu'au quadruple-clic, etc.

#### Mise à jour

```cpp
ButtonEvent update()
```

Fonction principale à appeler dans `loop()`. Retourne un événement de type `ButtonEvent`.

#### Getters

```cpp
uint8_t getClickCount()
```

Retourne le nombre de clics détectés lors du dernier événement `BUTTON_MULTI_CLICK`.

```cpp
unsigned long getPressDuration()
```

Retourne la durée du dernier appui en millisecondes.

```cpp
boolean isPressed()
```

Retourne `true` si le bouton est actuellement pressé.

### Événements (ButtonEvent)

- `BUTTON_NO_EVENT` : Aucun événement
- `BUTTON_PRESSED` : Bouton vient d'être pressé
- `BUTTON_RELEASED` : Bouton vient d'être relâché
- `BUTTON_SHORT_CLICK` : Appui court détecté (simple clic)
- `BUTTON_LONG_PRESS` : Appui long détecté
- `BUTTON_MULTI_CLICK` : Clics multiples détectés (utiliser `getClickCount()` pour connaître le nombre)

## Exemples

### Configuration pour différents types de boutons

**Bouton tactile TTP223 :**

```cpp
InputBouton button(D5, LOW);  // État repos = LOW, pressé = HIGH
```

**Bouton mécanique avec pull-up interne :**

```cpp
InputBouton button(D6, HIGH);  // État repos = HIGH, pressé = LOW
```

### Détection jusqu'à 4 clics

```cpp
button.setMaxClickCount(4);

ButtonEvent event = button.update();

if (event == BUTTON_MULTI_CLICK) {
  uint8_t clics = button.getClickCount();

  if (clics == 2) {
    // Double clic
  } else if (clics == 3) {
    // Triple clic
  } else if (clics == 4) {
    // Quadruple clic
  }
}
```

### Personnalisation des seuils

```cpp
// Appui court très rapide (max 200ms)
button.setShortPressMax(200);

// Appui long plus court (800ms au lieu de 1000ms)
button.setLongPressMin(800);

// Plus de temps entre les clics (500ms au lieu de 300ms)
button.setMultiClickTimeout(500);
```

## Structure des fichiers

```
lib/InputBouton/
├── InputBouton.h          # Fichier header
├── InputBouton.cpp        # Implémentation
├── README.md               # Cette documentation
└── examples/
    ├── BasicUsage/
    │   └── BasicUsage.ino   # Exemple simple
    └── AdvancedUsage/
        └── AdvancedUsage.ino # Exemple avancé avec multi-boutons
```

## Conseils d'utilisation

1. **Anti-rebond** : La librairie gère automatiquement les états, mais pour des boutons mécaniques très bruyants, vous pouvez ajuster les seuils.

2. **Appels dans loop()** : Appelez `button.update()` à chaque itération de `loop()` sans délai bloquant.

3. **Multiple boutons** : Créez une instance par bouton et appelez `update()` pour chacun.

4. **Performance** : La librairie utilise `millis()` et ne bloque jamais l'exécution.

## License

Libre d'utilisation pour vos projets personnels et commerciaux.
