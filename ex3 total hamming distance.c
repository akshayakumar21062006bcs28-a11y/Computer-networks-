int totalHammingDistance(int* nums, int numsSize) {
    int total = 0;

    for (int bit = 0; bit < 32; bit++) {
        int count1 = 0;

        for (int i = 0; i < numsSize; i++) {
            if ((nums[i] >> bit) & 1)
                count1++;
        }

        total += count1 * (numsSize - count1);
    }

    return total;
}