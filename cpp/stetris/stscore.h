#ifndef __STSCORE_H__
#define __STSCORE_H__

class CStScore
{
public:
    // Constructor
    CStScore();

    // Clear
    void Clear();

	// Add score
	void AddScore(unsigned int dwScore);
	// Set score
	void SetScore(unsigned int dwScore);
	// Get score
	unsigned int GetScore();

private:
	// Score
    unsigned int m_dwScore;
};

#endif
