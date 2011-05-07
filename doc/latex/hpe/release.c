    FILE *fp;

    /* Release input vectors */
    clFree(a);
    clFree(b);

    /* Write and release the output vector */
    fp = fopen(vector_c_file, "w");
    if(fp == NULL) {
        fprintf(stderr, "Cannot write output %s\n",
                vector_c_file);
        clFree(c); abort();
    }
    fwrite(c, vector_size, sizeof(float), fp);
    fclose(fp);
    clFree(c);