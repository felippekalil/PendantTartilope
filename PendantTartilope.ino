/*
 Name:		PendantTartilope.ino
 Created:	11/28/2019 12:30:02 PM
 Author:	Felippe Kalil Mendonça
*/

//#define MODO_ECO
//#define MODO_REPASSA

// inicializa
// clear -> '%' + '@' + 140
// enviaString (print) -> string
// moveCursor (scrollDisplayLeft, scrollDisplayRight) -> '<', '>'
// setaCursor (setCursor) -> novo
// leTeclado -> '&' + 92

#include <LiquidCrystal.h>
namespace Lcd
{
	//Inicializa o LCD
	LiquidCrystal lcd(A5, A4, A0, A1, A2, A3);

	void inicializar()
	{
		lcd.begin(40, 2);
		lcd.setCursor(0, 0);
		lcd.print("        Inicializando LCD...  ");/*
		lcd.print("TARTILOPE V2F - LABSOLDA - FINEP - SPS");
		lcd.setCursor(0, 1);
		lcd.print("   OPERAR      DESLIGAR       INFO    ");//*/
		delay(2000);
		lcd.clear();
	}

	void setaCursor(uint16_t cursor)
	{
		if(cursor < 0) cursor = 0;
		if(cursor > 78) cursor = 78;
		if(cursor < 40)
			lcd.setCursor(0, cursor);
		else
			lcd.setCursor(1, cursor - 40);
	}
}

namespace Botoes
{
	const int BUZZER = 3;
	const int BOTOES_FILA_1 = 4; //PD4
	const int BOTOES_FILA_2 = 5; //PD5
	const int BOTOES_FILA_3 = 6; //PD6
	const int BOTOES_ENTRADA_1 = 8; //PB0
	const int BOTOES_ENTRADA_2 = 9;	//PB1
	const int BOTOES_ENTRADA_3 = 10; //PB2
	const int BOTOES_ENTRADA_4 = 7;	//PD7

	void inicializar()
	{
		TCCR2B = (0 << FOC0A) | (0 << FOC0B) | (0 << WGM02) | // aumenta a frequencia do pwm, pra melhorar o som
			(0 << CS02) | (1 << CS01) | (0 << CS00);
		pinMode(BUZZER, OUTPUT);
		pinMode(BOTOES_FILA_1, OUTPUT);
		pinMode(BOTOES_FILA_2, OUTPUT);
		pinMode(BOTOES_FILA_3, OUTPUT);
		pinMode(BOTOES_ENTRADA_1, INPUT_PULLUP);
		pinMode(BOTOES_ENTRADA_2, INPUT_PULLUP);
		pinMode(BOTOES_ENTRADA_3, INPUT_PULLUP);
		pinMode(BOTOES_ENTRADA_4, INPUT_PULLUP);

		digitalWrite(BOTOES_FILA_1, false);
		digitalWrite(BOTOES_FILA_2, true);
		digitalWrite(BOTOES_FILA_3, true);

		analogWrite(BUZZER, 40);//*/
		delay(20);
		digitalWrite(BUZZER, false);
		delay(20);
		analogWrite(BUZZER, 40);//*/
		delay(20);
		digitalWrite(BUZZER, false);
	}

	byte leEntradas()
	{
		if(!digitalRead(BOTOES_ENTRADA_1)) return 1;
		if(!digitalRead(BOTOES_ENTRADA_2)) return 2;
		if(!digitalRead(BOTOES_ENTRADA_3)) return 3;
		if(!digitalRead(BOTOES_ENTRADA_4)) return 4;
		return 0;
	}

	void setaFilaBotoes(const byte fila)
	{
		if(fila == 1)
		{
			digitalWrite(BOTOES_FILA_1, false);
			digitalWrite(BOTOES_FILA_2, true);
			digitalWrite(BOTOES_FILA_3, true);
		}
		else if(fila == 2)
		{
			digitalWrite(BOTOES_FILA_1, true);
			digitalWrite(BOTOES_FILA_2, false);
			digitalWrite(BOTOES_FILA_3, true);
		}
		else if(fila == 3)
		{
			digitalWrite(BOTOES_FILA_1, true);
			digitalWrite(BOTOES_FILA_2, true);
			digitalWrite(BOTOES_FILA_3, false);
		}
	}

	char retornaBotao(const byte linha, const byte entrada)
	{
		char botao = 0;
		if(entrada < 1) return 0;
		if(linha == 1)
		{
			if(entrada == 1) botao = 'V';
			if(entrada == 2) botao = 'F';
			if(entrada == 3) botao = 'G';
			if(entrada == 4) botao = 'H';
		}
		else if(linha == 2)
		{
			//if (entrada == 1) botao = '';
			if(entrada == 2) botao = 'A';
			if(entrada == 3) botao = 'B';
			if(entrada == 4) botao = 'C';
		}
		else if(linha == 3)
		{
			if(entrada == 1) botao = 'D';
			if(entrada == 2) botao = 'J';
			if(entrada == 3) botao = 'E';
			if(entrada == 4) botao = 'K';
		}
		return botao;
	}

	char leBotoes()
	{
		// j� entra na fila 1
		auto botao = retornaBotao(1, leEntradas());
		if(botao > 0) return botao;
		setaFilaBotoes(2);
		delay(10);
		botao = retornaBotao(2, leEntradas());
		if(botao > 0)
		{
			setaFilaBotoes(1);
			return botao;
		}
		setaFilaBotoes(3);
		delay(10);
		botao = retornaBotao(3, leEntradas());
		setaFilaBotoes(1);
		return botao;
	}

	void apitaBuzzer(const uint32_t tempo)
	{
		//digitalWrite(BUZZER, true);/*
		analogWrite(BUZZER, 40);//*/
		delay(tempo);
		digitalWrite(BUZZER, false);
	}
}

namespace Buffer
{
	const int MAX_CONT_BUFFER = 200;
	byte bufferAcc[MAX_CONT_BUFFER];
	volatile uint8_t cont = 0;
	volatile bool dadoNovo = false;

	void removeBuffer()
	{
		memmove(bufferAcc, bufferAcc + 1, cont - 1);
		cont--;
	}

	void addBuffer(const byte dado)
	{
		if(cont >= MAX_CONT_BUFFER)
			removeBuffer();
		bufferAcc[cont++] = dado;
		dadoNovo = true;
	}
}

namespace Tratamento
{
	bool pegaBotao, segVoltar;

	void tratamento(const byte c)
	{
	#ifdef MODO_REPASSA
		if(c > 31 && c < 127)
			Lcd::lcd.print(c);
		else Lcd::lcd.command(c);
	#else
		if(pegaBotao)
		{
			pegaBotao = false;
			if(c == 92)
			{
				const auto botao = Botoes::leBotoes();
				if(botao > 0)
				{
					if(segVoltar && botao == 'V')
						Serial.write('I');
					else
						Serial.write(botao);
					Botoes::apitaBuzzer(15);
				}
				segVoltar = botao == 'V';
				return;
			}
			//lcd.print('&');
		}
		switch(c)
		{
			case '%':
				Lcd::lcd.clear();
				break;
			case '@':
			case 140:
				//lcd.home();
				Lcd::setaCursor(0);
				break;
			case '<':
			case 144:
				Lcd::lcd.command(0x10); // left shift cursor
				break;
			case '>':
			case 148:
				Lcd::lcd.command(0x14); // right shift cursor
				break;
			case '&':
				pegaBotao = true;
				break;
			case 7:
				Botoes::apitaBuzzer(150);
				delay(200);
				Botoes::apitaBuzzer(100);
				break;
			case 143:
				Botoes::apitaBuzzer(100);
				break;
			default:
				if(c > 31 && c < 127)
					Lcd::lcd.print(static_cast<char>(c));
				break;
		}
	#endif
	}

	void interpretaDados()
	{
		if(Buffer::cont > 0)
		{
		#ifdef MODO_ECO
			Serial.write(Buffer::bufferAcc[0]);
		#endif
			tratamento(Buffer::bufferAcc[0]);
			Buffer::removeBuffer();
		}
	}	
}

void setup()
{
	Serial.begin(9600);
	pinMode(LED_BUILTIN, OUTPUT);
	Botoes::inicializar();
	Lcd::inicializar();
}

void loop()
{
	Tratamento::interpretaDados();
}

void serialEvent()
{
	while(Serial.available())
		Buffer::addBuffer(static_cast<byte>(Serial.read()));
}
