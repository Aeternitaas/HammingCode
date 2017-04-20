#include <iostream>
#include <fstream>
#include <bitset>
#include <stdlib.h>
#include <time.h>

using namespace std;

/**
 * @author   Ket-Meng Cheng
 * @date     18 April 2017
 * 
 * Written for Operating Systems and Computer Architecture, Spring 2017 at Duquesne University.
 * 
 * This program reads "file.txt", encodes it using (7,4) Hamming Code, and generates
 * a "ofile.txt". Then, the program corrupts it purposefully, changing one bit inside
 * each codeword, saving the changes into "corrupted_file.txt". The decode method will then 
 * fix the corrupted codeword and then decode the file, saving it into "decoded.txt".
 *
 */

class Hamming 
{	
	// Set of masks.
	unsigned char b1 = 0x80,
		      b2 = 0x40,
		      b3 = 0x20,
		      b4 = 0x10,
		      b5 = 0x08,
		      b6 = 0x04,
		      b7 = 0x02,
		      b8 = 0x01;

	public:
		void encode(ifstream&, ofstream&);
		void decode();
		void corrupt(ifstream&);
	private:
		unsigned char encoder(unsigned char);
		unsigned char decoder(unsigned char);
};

/**
 *  Encodes a half-character.
 *
 *  @param   chara  unsigned half-character to be encoded.
 *  @return         returns an unsigned character that has been encoded via Hamming encoding.
 */
unsigned char Hamming::encoder(unsigned char chara)
{
	unsigned char bit5 = 0, 
		      bit6 = 0,
		      bit7 = 0,
		      codeword;

	chara = chara << 4;
	if ((chara & b1) != 0x00)
	{
		bit6++;
		bit7++;
	}
	if ((chara & b2) != 0x00)
	{
		bit5++;	
		bit7++;
	}
	if ((chara & b3) != 0x00)
       	{
		bit5++;
		bit6++;
	}
	if ((chara & b4) != 0x00)
	{
		bit5++;
		bit6++;
		bit7++;
	}

	// Modulo 2 to convert back into binary, then shift into place.
	bit5 = bit5 & 0x01;
	bit6 = bit6 & 0x01;
	bit7 = bit7 & 0x01;

	bit5 = bit5 << 3;
	bit6 = bit6 << 2;
	bit7 = bit7 << 1;

	// Add it into the character to form a codeword.
	codeword = chara ^ bit5 ^ bit6 ^ bit7;

	return codeword;
}

/**
 *  Fixes one-bit corruption in codewords.
 *
 *  @param   chara  unsigned codeword that has been potentially corrupted.
 *  @return         returns a half-character that has been de-corrupted.
 */
unsigned char Hamming::decoder(unsigned char chara)
{
	unsigned char syn1 = 0,
		      syn2 = 0,
		      syn3 = 0,
		      mask;
	int syndrome;

	// Generates the syndrome buffer.
	if ((chara & b1) != 0x00)
	{
		syn3++;
	}
	if ((chara & b2) != 0x00)
	{
		syn2++;
	}
	if ((chara & b3) != 0x00)
	{
		syn2++;
		syn3++;
	}
	if ((chara & b4) != 0x00)
	{
		syn1++;
	}
	if ((chara & b5) != 0x00)
	{
		syn1++;
		syn3++;
	}
	if ((chara & b6) != 0x00)
	{
		syn1++;
		syn2++;
	}
	if ((chara & b7) != 0x00)
	{
		syn1++;
		syn2++;
		syn3++;
	}

	// Shifts into place according to binary values and combines.
	//syn1 = syn1 % 2 << 2;
	//syn2 = syn2 % 2 << 1;
	//syn3 = syn3 % 2;
	
	syn1 = syn1 & 0x01;
	syn2 = syn2 & 0x01;
	syn3 = syn3 & 0x01;

	syn1 = syn1 << 2;
	syn2 = syn2 << 1;

	syndrome = syn1 ^ syn2 ^ syn3;

	mask = 0x01 << (8 - syndrome);
	chara = chara ^ mask;

	// Shifts down to standardize/align.
	chara = chara >> 4;

	return chara;
}

/**
 *  Reports the length of file.txt, then breaks it into two 4-bit long 
 *  data portions. It then calls encoder() which generates the Hamming
 *  Code for that byte.
 *
 *  @param  file  the read-in file.
 *  @param  ofile the write-to file.
 */
void Hamming::encode(ifstream& file, ofstream& ofile)
{
	char character;
	unsigned char set1, set2, chara;
	unsigned char codeword [2];

	// Reports the number of characters in the file.
	file.seekg(0, file.end);
	int length = file.tellg();
	file.seekg(0, file.beg);

	cout << "Your input is: " << endl;
	while (file.get(character))
	{
		chara = character;
		cout << chara;
		cout << " = " << bitset<8>(chara) << " == ";
		// b1 = first four, b2 = last four.
		character = chara;
		set1 = character >> 4;
		set2 = character << 4;
		set2 = set2 >> 4;
		cout << bitset<8>(set1) << " + " << bitset<8>(set2) << endl;
		codeword[0] = encoder(set1);
		codeword[1] = encoder(set2);
	
		// Writes codewords into ofile.txt.
		ofile << codeword[0] << codeword[1];
	}
	file.close();
	ofile.close();

	cout << "Length is: " << length << endl;
}

/**
 *  Corrupts ofile.txt, places the corrupted version into corrupted_file.txt,
 *  then feeds it into decoder() which checks and fixes errors, and reassembles 
 *  the character, placing the decoded message into decoded.txt.
 */
void Hamming::decode()
{
	char chara;
	unsigned char set1, set2, character;

	// Corrupts the file, placing the contents into corrupted_file.txt.
	ifstream file ("ofile.txt");
	corrupt(file);
	file.close();

	file.open ("corrupted_file.txt");
	ofstream ofile ("decoded.txt");
	if (file.is_open() && ofile.is_open()) 
	{
		while (file.get(chara))
		{
			// Reassembles the character from two bytes of codewords.
			set1 = decoder(chara);
			file.get(chara);
			set2 = decoder(chara);
			character = set1 << 4;
			character = character ^ set2;
			ofile << character;
		}
	}
	file.close();
	ofile.close();
}

/**
 *  Corrupts a file by switching one random bit inside of each character and outputs it 
 *  into corrupted_file.txt.
 *
 *  @param  file  the file to be corrupted.
 */
void Hamming::corrupt(ifstream& file)
{
	ofstream cfile ("corrupted_file.txt");
	char chara;
	unsigned char corrupt, corrupted, mask;
	int random, count = 0;

	if (cfile.is_open() && file.is_open()) 
	{
		while (file.get(chara))
		{	
			// Corrupts a random bit.
			random = rand() & 0x07;
			corrupt = chara;
			
			mask = 0x01 << random;
			corrupt = corrupt ^ mask;

			count++;
			cfile << corrupt;
		}
	}
	cfile.close();
	cout << "Corruption complete, please check corrupted_file.txt" << endl << "# of characters in the new file: " << count << endl;
}

int main()
{
	char chara;
	unsigned char b1, b2, character;
	unsigned char codeword [2];
	// Hint:
	//   ifstream = input stream
	//   ofstream = output stream
	ifstream file ("file.txt");
	ofstream ofile ("ofile.txt");
	Hamming hamm;
	srand (time(0));

	if (file.is_open() && ofile.is_open())
	{
		// Seeks to end
		hamm.encode(file, ofile);
		hamm.decode();
	}
	else 
		cout << "There was an issue opening the file. Perhaps there is an issue with permissions?" << endl;

}
